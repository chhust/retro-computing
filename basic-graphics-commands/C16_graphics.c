// Some graphics commands vaguely inspired by Commodore BASIC V3.5 for the Commodore 16:
//
// GRAPHIC . sets the resolution mode
// SCNCLR .. clears screen by setting all pixels white
// DRAW .... draws dots or lines
// BOX ..... draws rectangles
// PAINT ... fills an area (I renamed this)
// LOCATE .. sets the graphics cursor
//
// DRAW originally allowed passing multiple parameters, eg. DRAW color, x0,y0, x1,y1, x2,y2, to draw complex shapes.
// This is emulated by implementing the coordinates as a linked list of "coordinate" structures, using a second function DRAW_from_list.
//
// Other commands included CIRCLE (drawing circles or ellipses or segments of them: this looks really hard to do)
//                     and COLOR  (defining the color from a fixed palette of color/brightness values),
// but those are not implemented here.
//
// The demos use the C16's max screen resolution of 320 x 200 pixels (how impressive...).
// As the C16 had only 16 KB, graphics information was stored differently: Color information was stored by storing a color ID 
// and a brightness ID of a few bits each, not multi-byte RGB data. The lo-res mode had further restrictions concerning
// how many colors might be used within one square of 8 x 8 pixels, resulting in less graphics memory that had to be reserved.
// My program uses 320 x 200 = 64,000 pixels à 3 char values à 1 byte, resulting in 192,000 bytes or 187,5 KB (let alone linked lists etc.).
// This is more than 11 times the complete RAM of C16, and still nearly 3 times the RAM of C64!
//
// I found a BASIC / assembly version of the line-drawing algorithm in an old book on computer graphics
// (Klaus Loeffelmann, Axel Plenge: "Das Grafikbuch zum Commodore 16", Duesseldorf 1986) and simply translated in to C.
//
// This program could be enhanced into a direct mode BASIC emulator. I would first parse the input (building from the parser in the diary program),
// then build syntax trees (building from the infix to postfix converter), update or insert variable values (using a hashtable),
// prepare the data (structs or linked lists), and finally execute the command.
//
// There are books on interpeter building, like Bob Nystrom, "Crafting Interpreters"; Terence Parr, "Language Implementation Patterns";
//                                           or Daniel Friedman and Mitchell Wand, "Essentials of Programming Languages".

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#pragma pack(push, 1)

typedef struct {
    unsigned char signature[2];                                                     // 2 bytes: signature, should be "BM"
    unsigned int file_size;                                                         // 4 bytes: file size
    unsigned int reserved;                                                          // 4 bytes: reserved, should be 0
    unsigned int data_offset;                                                       // 4 bytes: offset of image data
} file_header;

typedef struct {
    unsigned int size;                                                              // 4 bytes: size of bitmap info header
    int width, height;                                                              // 8 bytes: width and height (4 bytes each)
    unsigned short planes;                                                          // 2 bytes: planes, should be 1
    unsigned short bit_count;                                                       // 2 bytes: color depth, can be 1, 4, 8, 16, 24, or 32
    unsigned int compression;                                                       // 4 bytes: compression, should be 0
    unsigned int image_size;                                                        // 4 bytes: size of image data
    int x_ppm, y_ppm;                                                               // 8 bytes: pixel per meter for x and y, both should be 0
    unsigned int clr_used;                                                          // 4 bytes: color palette, should be 0
    unsigned int clr_important;                                                     // 4 bytes: important colors, should be 0
} info_header;

#pragma pack(pop)

typedef struct {                                                                    // Color information for a single pixel
    unsigned char b, g, r;
} RGB_data;

typedef struct {                                                                    // Set of X/Y coordinates
    int x, y;
} coordinates;

typedef struct parameter_list {
    coordinates xy;
    struct parameter_list* next;
} parameter_list;

typedef struct {
    int width, height;
} resolution;


// Function prototypes: GRAPHIC ........ set screen resolution
//                      SCNCLR ......... clear screen
//                      DRAW ........... draw a line, using two pair of coordinates (from -> to)
//                      DRAW_from_list . draw a line, taking coordinates from a linked list
//                      PAINT .......... fill an area, return number of pixels
void GRAPHIC(int mode, resolution* screen);
void SCNCLR(RGB_data* bitmap, resolution screen);
void DRAW(coordinates start, coordinates end, RGB_data color, coordinates* graphics_cursor, RGB_data* bitmap, resolution screen);
void DRAW_from_list(parameter_list* head, RGB_data color, coordinates* graphics_cursor, RGB_data* bitmap, resolution screen);
void BOX(coordinates start, coordinates end, RGB_data color, int angle, bool fill, coordinates* graphics_cursor, RGB_data* bitmap, resolution screen);
int  PAINT(coordinates start, RGB_data target_color, RGB_data fill_color, RGB_data* bitmap, resolution screen);
void LOCATE(coordinates new, coordinates* graphics_cursor, resolution screen);

// Utility functions:   line drawing algorithm
//                      linked list management (add element, delete list)
//                      box corner rotation
//                      color check
//                      file save
void draw_line(coordinates from, coordinates to, RGB_data color, RGB_data* bitmap, resolution screen);
void add_coordinates_to_list(parameter_list **head, int x, int y);
void free_coordinates_list(parameter_list* head);
coordinates rotate(coordinates point, int angle, coordinates pivot);
bool same_color(RGB_data color1, RGB_data color2);
void save_BMP(const char *filename, RGB_data* bitmap, resolution screen);


// main is simply a succession of demo routines.

int main() {
    printf("Graphics demo emulating the 320 x 200 pixel 'hi-res' mode of the Commodore 16.\n\n");
    resolution screen;                                                              // Declare resolution variable
    GRAPHIC(1, &screen);                                                            // 1 and 2: Hi-res resolution of 320 x 200 pixels
    RGB_data* bitmap = (RGB_data *) malloc(screen.width * screen.height * sizeof(RGB_data));
    RGB_data current_color = {0xFF, 0x00, 0x00};                                    // Current color for drawing
    RGB_data target_color  = {0xFF, 0xFF, 0xFF};                                    // Color that will be filled
    RGB_data fill_color1   = {0x00, 0xFF, 0x00};                                    // Color 1 to fill an area with
    RGB_data fill_color2   = {0xFF, 0xFF, 0x00};                                    // Color 2 to fill an area with
    coordinates* graphics_cursor = malloc(sizeof(coordinates));                     // Graphics cursor
    if(!graphics_cursor || !bitmap) {
        printf("Memory allocation failed.\n");
        return 1;
    }
    graphics_cursor->x = 0, graphics_cursor->y = 0;
    coordinates from, to, start;                                                    // Coordinates for DRAW and FILL commands

    printf("Creating demo picture 1 ... ");                                         // Create demo picture 1
    SCNCLR(bitmap, screen);
    from.x = 001, from.y = 001, to.x =  -1, to.y =  -1;
    DRAW(from, to, current_color, graphics_cursor, bitmap, screen);                 // Draw single dot
    from.x = 010, from.y = 010, to.x = 300, to.y = 180;
    DRAW(from, to, current_color, graphics_cursor, bitmap, screen);                 // Draw line with from-to coordinates

    parameter_list* head = NULL;                                                    // Draw some more lines from linked list
    add_coordinates_to_list(&head,  -1,  -1);
    add_coordinates_to_list(&head, 100, 100);
    add_coordinates_to_list(&head, 010,  010);
    DRAW_from_list(head, current_color, graphics_cursor, bitmap, screen);
    free_coordinates_list(head);
    save_BMP("output1.bmp", bitmap, screen);
    printf("Done.\n");

    printf("Creating demo picture 2 ... ");                                         // Create demo picture 2
    SCNCLR(bitmap, screen);
    to.x = screen.width, to.y = screen.height;
    for(int i = 0; i < screen.height; i += 25) {                                    // Draw some lines
        from.x = 0, from.y = i;
        DRAW(from, to, current_color, graphics_cursor, bitmap, screen);
    }
    for(int i = 0; i < screen.width; i += 25) {
        from.x = i, from.y = 000;
        DRAW(from, to, current_color, graphics_cursor, bitmap, screen);
    }
    save_BMP("output2.bmp", bitmap, screen);
    printf("Done.\n");

    printf("Creating demo picture 3 ... ");                                         // Create demo picture 3
    for(int i = 10; i < screen.height; i += 50) {                                   // Fill every other space between lines
        start.x = 000, start.y = i;
        PAINT(start, target_color, fill_color1, bitmap, screen);
    }
    for(int i = 30; i < screen.width; i += 50) {
        start.x = i, start.y = 0;
        PAINT(start, target_color, fill_color1, bitmap, screen);
    }
    save_BMP("output3.bmp", bitmap, screen);
    printf("Done.\n");

    printf("Creating demo picture 4 ... ");                                         // Create demo picture 4
    SCNCLR(bitmap, screen);
    from.x = 050, from.y = 050, to.x = 150, to.y = 150;
    BOX(from, to, current_color, 5, 0, graphics_cursor, bitmap, screen);            // Rotation 5°, not filled
    start.x = 000, start.y = 000;
    PAINT(start, target_color, fill_color1, bitmap, screen);                        // Fill outside
    start.x = 100, start.y = 100;
    PAINT(start, target_color, fill_color2, bitmap, screen);                        // Fill inside
    save_BMP("output4.bmp", bitmap, screen);
    printf("Done.\n");

    SCNCLR(bitmap, screen);                                                         // Demo 5: show pixel count (picture will not be saved)
    printf("Filling the complete screen takes %d pixels.\n", PAINT(start, target_color, fill_color2, bitmap, screen));

    free(graphics_cursor);
    free(bitmap);
    return 0;
}


// Sets the graphic mode to hi-res or lo-res

void GRAPHIC(int mode, resolution* screen) {
    switch(mode) {
        case 0:                                                                     // Graphics mode off, pure text mode
            printf("Text mode not available.\n");
            exit(1);
        case 1:                                                                     // Hi-res mode
        case 2:                                                                     // Hi-res mode plus text
            screen->width  = 320;
            screen->height = 200;
            break;
        case 3:                                                                     // Lo-res mode
        case 4:                                                                     // Lo-res mode plus text
            screen->width  = 160;
            screen->height = 200;
            break;
        default:
            printf("Resolution error.\n");
            exit(1);
    }
}


// Implements the SCNCLR command:
// Iterates complete bitmap and fill with white pixels

void SCNCLR(RGB_data* bitmap, resolution screen) {
    for (int i = 0; i < screen.width * screen.height; i++) {
        bitmap[i].r = 0xFF;
        bitmap[i].g = 0xFF;
        bitmap[i].b = 0xFF;
    }
}


// Implements the DRAW command:
// -- if start == -1, use current graphics cursor position as starting point
// -- if end   == -1, draw a dot at start / graphics cursor position
// -- otherwise,      draw a line from start to end and update graphics cursor position

void DRAW(coordinates start, coordinates end, RGB_data color, coordinates* graphics_cursor, RGB_data* bitmap, resolution screen) {
    if (start.x == -1 || start.y == -1) {                                           // Check if starting point is {-1, -1}
        start.x = graphics_cursor->x;
        start.y = graphics_cursor->y;
    }

    if (end.x == -1 && end.y == -1) {                                               // Check if ending point is {-1, -1}
        end.x = start.x;
        end.y = start.y;
    }

    draw_line(start, end, color, bitmap, screen);                                   // Draw
    LOCATE(end, graphics_cursor, screen);                                           // Update graphics cursor
}


// Draw a shape indicated by a linked list of coordinates.

void DRAW_from_list(parameter_list* head, RGB_data color, coordinates* graphics_cursor, RGB_data* bitmap, resolution screen) {
    if (head == NULL || head->next == NULL) {                                       // Empty list
        return;
    }

    parameter_list* current = head;                                                 // Start at beginning,

    if (current->xy.x != -1 && current->xy.y != -1) {                               // Check first element for -1,-1 (= use current position)
        LOCATE(current->xy, graphics_cursor, screen);                               // and set graphics cursor
        current = current->next;                                                    // Move to the second node
    }

    while (current != NULL && current->next != NULL) {                                  // iterate list,
        DRAW(current->xy, current->next->xy, color, graphics_cursor, bitmap, screen);   // draw line, starting at graphics cursor position,
        current = current->next;                                                        // process next element.
    }
}


// Draw a box (rectangle)
// "fill" parameter is passed but not processed. I would need to calculate a starting point within the rectangle,
// then modify PAINT so that every color that is not equal to the border color of the rectangle will be overwritten.

void BOX(coordinates start, coordinates end, RGB_data color, int angle, bool fill, coordinates* graphics_cursor, RGB_data* bitmap, resolution screen) {
    coordinates corners[4] = {                                                      // Determine the four corner points
        start,
        {end.x, start.y},
        end,
        {start.x, end.y}
    };

    if(angle) {                                                                     // If necessary, rotate corner points
        for(int i = 0; i < 4; i++) {
            corners[i] = rotate(corners[i], angle, start);
        }
    }

    for(int i = 0; i < 4; i++) {                                                        // Draw border lines
        DRAW(corners[i], corners[(i + 1) % 4], color, graphics_cursor, bitmap, screen); // (i + 1) % 4 closes the rectangle by going back to corner 0
    }
}


// This fills a certain area of adjacent pixels in "target_color" by updating them to "fill_color".

int PAINT(coordinates start, RGB_data target_color, RGB_data fill_color, RGB_data* bitmap, resolution screen) {
    if(start.x < 0 || start.x >= screen.width || start.y < 0 || start.y >= screen.height) {     // Check for boundaries
        return 0;
    }

    if(!same_color(bitmap[start.y * screen.width + start.x], target_color)) {       // Don't do anything if current pixel is not in target_color
        return 0;
    }

    coordinates* stack = malloc(screen.width * screen.height * sizeof(coordinates));    // Create a stack of coordinates which will
    if(!stack) {                                                                        // keep track of pixels that still have to be filled
        printf("Unable to allocate memory for stack.\n");
        return 0;
    }
    int stack_size = 0;
    int filled_pixels = 0;                                                          // Counter for filled pixels; this will be the return value

    stack[stack_size++] = start;                                                    // Enqueue starting point

    while(stack_size) {                                                             // Do until stack is empty
        coordinates pixel = stack[--stack_size];                                    // Dequeue next pixel to be processed

        if(!same_color(bitmap[pixel.y * screen.width + pixel.x], target_color)) {   // Skip if pixel is not in target color
            continue;
        }

        int fill_left  = pixel.x;                                                   // Initialize variables for left and right boundaries
        int fill_right = pixel.x;

        while(fill_left > 0 && same_color(bitmap[pixel.y * screen.width + fill_left - 1], target_color)) {  // Go to left until either left boundary
            fill_left--;                                                            // or a pixel in a different color will be reached
        }

        while(fill_right < screen.width - 1 && same_color(bitmap[pixel.y * screen.width + fill_right + 1], target_color)) { // Vice versa for right
            fill_right++;
        }

        for(int i = fill_left; i <= fill_right; ++i) {                              // Fill the current line, check below and above
            if(same_color(bitmap[pixel.y * screen.width + i], target_color)) {      // If correct color, fill and count
                bitmap[pixel.y * screen.width + i] = fill_color;
                filled_pixels++;
            }

            if(pixel.y > 0 && same_color(bitmap[(pixel.y - 1) * screen.width + i], target_color)) {     // Check and enqueue lines below and above
                stack[stack_size++] = (coordinates) {i, pixel.y - 1};
            }

            if(pixel.y < (screen.height - 1) && same_color(bitmap[(pixel.y + 1) * screen.width + i], target_color)) {
                stack[stack_size++] = (coordinates){i, pixel.y + 1};
            }
        }
    }

    free(stack);
    return filled_pixels;                                                           // Returning the number of pixels processed is a bonus ;)
}


// Sets the graphics cursor to a certain point; this is trivial.

void LOCATE(coordinates new, coordinates* graphics_cursor, resolution screen) {
    if(new.x < 0 || new.x >= screen.width || new.y < 0 || new.y >= screen.height) {
        return;
    }
    graphics_cursor->x = new.x;
    graphics_cursor->y = new.y;
}


// Bresenham algorithm for drawing lines.

void draw_line(coordinates from, coordinates to, RGB_data color, RGB_data* bitmap, resolution screen) {
    int dx =  abs(to.x - from.x), sx = from.x < to.x ? 1 : -1;                      // Difference and sign, x axis (dx, sx)
    int dy = -abs(to.y - from.y), sy = from.y < to.y ? 1 : -1;                      // Same for y axis
    int error = dx + dy, temp_error;                                                // Initial error value and temp error declaration

    while (1) {
        if(from.x >= 0 && from.x < screen.width && from.y >= 0 && from.y < screen.width) {  // Check for boundaries
            // Set next point of line:
            // from.y * screen.width "fast forwards" complete lines,
            // from.x adds until we reach the correct x axis position
            bitmap[from.y * screen.width + from.x] = color;
        }
        if(from.x == to.x && from.y == to.y) {                                      // Check whether ending point has been reached
            break;
        }

        // dx and dy  are the absolute differences in the x and y coordinates of the line's endpoints, 
        //            representing the "length" of the line along each axis.
        // error      is initialized as dx + dy, representing the cumulative error in the line drawing.
        // temp_error is used to compare the error against dx and dy. It is doubled to avoid floating-point calculations.
        // if(temp_error >= dy) checks if error*2 is at least as large as dy, meaning the line is deviating in x direction
        //                      If true, x position is incremented to bring the line nearer its ideal position.
        //                      Error is updated to bring it back towards zero
        // if(temp_error <= dx) works exactly vice versa
        // These conditions and updates ensure that the line progresses steadily in both x and y directions, depending on the slope.
        // The algorithm essentially decides whether to move horizontally, vertically, or diagonally at each step,
        // based on how far the current path has deviated from the ideal line.
        temp_error = 2 * error;                                                     // Store 2 * error in temp variable

        if(temp_error >= dy) {
            from.x += sx;
            error  += dy;
        }

        if(temp_error <= dx) {
            from.y += sy;
            error  += dx;
        }
    }
}


// Adds a new set of coordinates to the coordinates linked list

void add_coordinates_to_list(parameter_list** head, int x, int y) {
    parameter_list* new_node = malloc(sizeof(parameter_list));                      // Allocate memory for new node
    if(!new_node) {
        printf("Memory allocation failed.\n");
        return;
    }

    new_node->xy.x = x;                                                             // Enter data
    new_node->xy.y = y;
    new_node->next = NULL;                                                          // Enter NULL pointer

    if(*head == NULL) {                                                             // First node? if yes, set it as head
        *head = new_node;
    } else {
        parameter_list* current = *head;                                            // If not, iterate the list
        while(current->next) {
            current = current->next;
        }
        current->next = new_node;                                                   // and add new node at the end
    }
}


// Frees the memory of the list of coordinates

void free_coordinates_list(parameter_list* head) {
    parameter_list* temp;
    while(head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}


// Rotate coordinates. This looks tricky but is standard code.

coordinates rotate(coordinates point, int angle, coordinates pivot) {
    double radians = angle * M_PI / 180;
    int x = round(cos(radians) * (point.x - pivot.x) - sin(radians) * (point.y - pivot.y) + pivot.x);
    int y = round(sin(radians) * (point.x - pivot.x) + cos(radians) * (point.y - pivot.y) + pivot.y);
    return (coordinates){x, y};
}


// Compares color1 and color2; this is trivial.

bool same_color(RGB_data color1, RGB_data color2) {
    return color1.r == color2.r && color1.g == color2.g && color1.b == color2.b;
}


// BMP saving

void save_BMP(const char *filename, RGB_data* bitmap, resolution screen) {
    FILE *file = fopen(filename, "wb");
    if(!file) {
        printf("Unable to open file %s.\n", filename);
        return;
    }

    file_header bmp_header = {
        { 'B', 'M' },                                                               // Signature: "BM"
        54 + screen.width * screen.height * sizeof(RGB_data),                       // File size is header size plus data size
        0,                                                                          // Reserved (should be 0)
        54                                                                          // Data offset: image data will start after headers
    };

    info_header bmp_info_header = {
        40,                                                                         // Size of info header
        screen.width,                                                               // Width of picture
        -screen.height,                                                             // Height; negative value indicates origin in top-left corner
        1,                                                                          // Planes
        24,                                                                         // BitCount (24-bits BMP file)
        0,                                                                          // Compression
        screen.width * screen.height * sizeof(RGB_data),                            // Size of image data
        0, 0,                                                                       // Horizontal and vertical resolution in pixels per meter
        0,                                                                          // ClrUsed = 0 means standard values
        0                                                                           // ClrImportant = 0 is the standard setting
    };

    fwrite(&bmp_header, sizeof(file_header), 1, file);                              // Write file header
    fwrite(&bmp_info_header, sizeof(info_header), 1, file);                         // Write info header
    fwrite(bitmap, sizeof(RGB_data), screen.width * screen.height, file);           // Write bitmap data
    fclose(file);                                                                   // Close
}
