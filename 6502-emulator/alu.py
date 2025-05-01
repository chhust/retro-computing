"""
6502 ARITHMETIC-LOGIC UNIT EMULATION
====================================

This code is a gate-level emulation of the ADC (add with carry) function, adding two numbers.
Below gate-level, transistors are already defined for a future descent one level down.
Gates are represented as nodes in a graph structure, with edges representing tracks.

Most of this is done from scratch, with some magic black-box like parts still left where I actually only simulate
outcomes, not actual mechanics.

For the microelectronics parts (the deeper levels), I used two books:
+ the more technical and extensive "Digital Design and Computer Architecture" by David Money Harris and Sarah L.
  Harris (2nd ed., Burlington: Morgan Kaufmann, 2012),
+ and the super-accessible "CODE: The Hidden Language of Computer Hardware and Software" by Charles Petzold (I used
  the German translation, "CODE: Wie Computer funktionieren", 2nd ed., Frechen: mitp, 2023).

I took my ideas about the processor from the 6502 documentation and literature, mostly from the wonderful book by
+ Rodnay Zaks, "Programming the 6502" that includes some technical overview data as well (again, I used a German
  translation: "Programmierung des 6502", 3rd ed., Düsseldorf: Sybex, 1982).
"""

import time
TIMER = .25


# Lowest level: a single transistor, not yet connected to the higher levels.

class Transistor:
    """
    This class simulates one single transistor.
    To include this, eg. an ANDNode would be built by two NMOS transistors in series.
    """

    def __init__(self, name, type, source, drain, gate):
        self.name = name        # transistor name
        self.type = type        # transistor type: NMOS or PMOS
        self.source = source    # source
        self.drain = drain      # drain
        self.gate = gate        # gate input signal
        self.state = False      # state: on/conducting or off/non-conducting

    def evaluate(self, inputs):
        """
        Evaluate transistor based on input signals.
        """

        if self.type == "NMOS":
            self.state = True if inputs[self.gate] == True else False   # conducts when gate is high
        elif self.type == "PMOS":
            self.state = True if inputs[self.gate] == False else False  # conducts when gate is low

        return self.state


# Next level: logic gates (as nodes for the graph).

class LogicNode:
    """
    This class defines logic gate nodes: AND, OR, XOR, NOT.
    Each has named inputs, and each produces one single output.
    """

    def __init__(self, name):
        self.name = name
        self.inputs = []
        self.output = 0

    def add_input(self, input_node):
        self.inputs.append(input_node)

    def evaluate(self, inputs):
        """
        Evaluate the logic gate based on the current input values.
        The method is implemented by subclasses; this is an error message for debugging.
        """

        raise NotImplementedError("Must be implemented in subclass")


# What follows are classes for specific logic gate nodes, corresponding to the LogicNode class

class ANDNode(LogicNode):
    """
    AND gate: Output is 1 if all inputs are 1.
    """

    def evaluate(self, inputs):
        self.output = 1 if all(inputs[i] for i in self.inputs) else 0

        return self.output

class ORNode(LogicNode):
    """
    OR gate: Output is 1 if at least one input is 1.
    """

    def evaluate(self, inputs):
        self.output = 1 if any(inputs[i] for i in self.inputs) else 0

        return self.output

class XORNode(LogicNode):
    """
    XOR gate: Output is 1 if inputs differ (one is 1, one is 0).
    """

    def evaluate(self, inputs):
        a, b = self.inputs
        self.output = (inputs[a] ^ inputs[b])

        return self.output

class NOTNode(LogicNode):
    """
    NOT gate: Output is the opposite of the (single) input.
    """

    def evaluate(self, inputs):
        self.output = 1 if inputs[self.inputs[0]] == 0 else 0

        return self.output


# Next level: a full adder built from various logic gates

class FullAdder:
    """
    sum is defined as a XOR b XOR carry_in
    carry_out is defined as (a AND b) or ((a XOR b) AND carry_in)

           a -------+
                    |--- XOR ---+
           b -------+           |
                                |--- XOR ------ sum
        carry_in ---------------+

           a -------+
                    |--- AND ---+
           b -------+           |
                                |--- OR ------- carry_out
     (a XOR b) ---- AND --------+
    """

    def __init__(self):
        # Start by setting up the logic nodes we need:
        # XOR for sum; AND + OR for the carry flag
        self.nodes = {
            "xor1": XORNode("xor1"),
            "xor2": XORNode("xor2"),
            "and1": ANDNode("and1"),
            "and2": ANDNode("and2"),
            "or1" : ORNode("or1")
        }
        
        # Next, define the input connections to build the graph
        self.nodes["xor1"].inputs = ["a", "b"]
        self.nodes["xor2"].inputs = ["xor1_output", "carry_in"]
        self.nodes["and1"].inputs = ["a", "b"]
        self.nodes["and2"].inputs = ["xor1_output", "carry_in"]
        self.nodes["or1"].inputs  = ["and1_output", "and2_output"]


    def evaluate(self, a, b, carry_in):
        """
        Look for given input bits a, b, carry_in
        """
        inputs = {"a": a, "b": b, "carry_in": carry_in}
        
        # Pass signals through first XOR gate, store result
        inputs["xor1_output"] = self.nodes["xor1"].evaluate(inputs)
        
        # Pass through second XOR gate for the sum_bit
        sum_bit = self.nodes["xor2"].evaluate(inputs)
        
        # Pass signals through two AND gates and store the results
        inputs["and1_output"] = self.nodes["and1"].evaluate(inputs)
        inputs["and2_output"] = self.nodes["and2"].evaluate(inputs)
        
        # Pass signals through OR gate for the carry_out flag
        carry_out = self.nodes["or1"].evaluate(inputs)
        
        return sum_bit, carry_out


# Final step: an 8 bit ALU (Arithmetic-Logical Unit) with adders for the ADC command as all it can do. :)
# It even has the 6502 BCD ("binary mode") superficially implemented, hopefully it's correct...
# Right now, the code mixes the actual calculation with control output -> should be separated

class ALU:
    def __init__(self):
        # I need one instance from the FullAdder class per bit.
        self.adders = [FullAdder() for _ in range(8)]
        
    def add_8bit(self, num1, num2, carry_in=0, decimal_mode=False):
        # Split an 8 bit number into eight individual bits.
        a_bits = [(num1 >> i) & 1 for i in range(8)]
        b_bits = [(num2 >> i) & 1 for i in range(8)]
        
        result = 0
        carry = carry_in
        
        print(f"Starting 8-bit addition of 0b{num1:08b} and 0b{num2:08b}\n")
        print("-" * 57)
        print(f"Bit Position | Bit A | Bit B | Sum | Carry In | Carry Out")
        print("-" * 57)
        
        # Do the addition, bit by bit
        for i in range(8):
            current_carry_in = carry
            sum_bit, carry = self.adders[i].evaluate(a_bits[i], b_bits[i], carry)
            result |= (sum_bit << i)        # shift to correct position and blend
            
            # Control output
            print(f"      {i}      |   {a_bits[i]}   |   {b_bits[i]}   |  {sum_bit}  |     {current_carry_in}    |     {carry}")
            time.sleep(TIMER)   # the delay adds extra drama :)
        
        # Check for decimal mode, and apply the correction if needed (helper function below)
        if decimal_mode:
            result = bcd_correction(result)
        
        # Update the flags array
        zero_flag = result == 0

        negative_flag = (result >> 7) & 1 == 1

        sign_a = a_bits[7]
        sign_b = b_bits[7]
        sign_res = (result >> 7) & 1
        overflow_flag = ((sign_a ^ sign_res) & (sign_b ^ sign_res)) & 1
        # Or, the unreadable version:
        # overflow_flag = ((a_bits[7] ^ result >> 7) & (b_bits[7] ^ result >> 7)) & 1

        carry_flag = carry
        
        # Show the end result
        print(f"\nFinal Result     - 0b{result:08b} ({hex(result)}, {result})")
        print(f"Processor Status - C {bool(carry_flag)} | Z {bool(zero_flag)} | N {bool(negative_flag)} | O {bool(overflow_flag)}")
        
        return result, carry_flag, zero_flag, negative_flag, overflow_flag


# Lastly, one helper function for BCD:

def bcd_correction(result):
    """
    Applies BCD (Binary-Coded Decimal) correction to an 8 bit result.
    Adds 6 to any nibble (4 bits) > 9 to ensure valid BCD:
    In BCD, each nibble encodes one decimal digit (0 to 9).
    So the decimal number 42 would be stored as 0100 0010
                                        (not as 0010 1010 which would be usual binary).
    I have not yet checked how this is actually implemented in the 6502,
    and this only simulated the outcome, not the actual mechanics.
    """

    low_nibble = result & 0x0F              # bit masking
    high_nibble = (result >> 4) & 0x0F
    if low_nibble > 9:
        result += 6
    if high_nibble > 9:
        result += 0x60

    return result


def main():
    print("6502 ARITHMETIC-LOGIC UNIT EMULATION")
    print("====================================\n")
    print("This code is a gate-level emulation of the ADC function, adding two numbers.")
    print("Gates are represented as nodes in a graph structure, with edges representing tracks.\n")
        
    num1 = 170      # 1010 1010
    num2 =  85      # 0101 0101         result should be 1111 1111
    carry_in = 0
    decimal_mode = False

    alu = ALU()
    alu.add_8bit(num1, num2, carry_in, decimal_mode)

if __name__ == "__main__":
    main()
