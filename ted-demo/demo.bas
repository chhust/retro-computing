5 rem  ******************************
6 rem  * commodore 264 series / ted *
7 rem  * (mos 7360/8360) sound demo *
8 rem  ******************************
10 rem
15 rem this code allows experiments with
20 rem the ted chip (mos 7360/8360).
25 rem users can control its 10-bit re-
30 rem soluted voices, volume control,
35 rem and modes (on, off; rectangle
40 rem waves and noise).
45 rem
50 rem the code addresses control regis-
55 rem ters directly via peek/poke com-
60 rem mands.
65 rem
70 rem it demonstrates ted's unequal
75 rem step width, noise-synthesis algo-
80 rem rithm, and beating effects.
85 rem
90 rem chh, 19.07.2024
95 rem
100 scnclr
110 print "cbm 264 series ted sound demo"
120 print "-----------------------------"
130 print
140 print "hit q, a for pitch control 1"
150 print "   (combine with shift for wider steps)"
160 print "hit o, k for pitch control 2"
170 print "   (combine with shift for wider steps)"
180 print "hit i to enter pitches manually"
190 print "hit v, b to toggle channels"
200 print "hit number keys for volume control"
210 print "hit space to toggle channel 2 mode"
220 print "hit esc to exit"
230 gosub 1000
240 rem main loop
250 do
260 get k$
270 kn = asc(k$)
280 if kn = 27 then gosub 2000
290 if kn = 65 or kn = 81 then gosub 3000
300 if kn = 193 or kn = 209 then gosub 3000
310 if kn = 75 or kn = 79 then gosub 4000
320 if kn = 203 or  kn = 207 then gosub 4000
330 if kn = 32 then gosub 5000
340 if kn >= 48 and kn <= 56 then gosub 6000
350 if kn = 86 then gosub 7000
360 if kn = 66 then gosub 8000
370 if kn = 73 then gosub 9000
380 loop
997 rem  ***********************
998 rem  * data initialization *
999 rem  ***********************
1000 v = 0
1010 c1 = 100
1020 c2 = 200
1030 poke 65294, c1 and 255
1040 poke 65298, (peek(65298) and 252) or (int(c1 / 256) and 3)
1050 poke 65295, c2 and 255
1060 poke 65296, int(c2 / 256) and 255
1070 n = 0 : rem flag for noise production
1080 vol 0
1090 poke 205,13 : poke 202,0 : sys 55464
1100 print chr$(18); "pitch 1  "; chr$(146)
1110 poke 205,13 : poke 202,12 : sys 55464
1120 print chr$(18); "pitch 2  "; chr$(146)
1130 poke 205,13 : poke 202,24 : sys 55464
1140 print chr$(18); "volume   "; chr$(146)
1150 print using "####"; c1;
1160 print " "; hex$(c1); "   ";
1170 print using "####"; c2;
1180 print " "; hex$(c2); "   ";
1190 print using "#"; v
1200 poke 205,16 : poke 202,0 : sys 55464
1210 print chr$(18); "channel 1"; chr$(146)
1220 print "off"
1230 poke 205,16 : poke 202,12 : sys 55464
1240 print chr$(18); "channel 2"; chr$(146)
1250 poke 205,17 : poke 202,12 : sys 55464
1260 print "off"
1270 poke 205,19 : poke 202,12 : sys 55464
1280 print chr$(18); "mode     "; chr$(146)
1290 poke 205,20 : poke 202,12 : sys 55464
1300 print "sound"
1310 return
1997 rem ****************
1998 rem * quit program *
1999 rem ****************
2000 vol 0
2010 scnclr
2020 end
2997 rem ***************************
2998 rem * channel 1 pitch control *
2999 rem ***************************
3000 if kn = 65 and c1 > 0 then c1 = c1 - 1
3010 if kn = 81 and c1 < 1023 then c1 = c1 + 1
3020 if kn = 193 and c1 >= 10 then c1 = c1 - 10
3030 if kn = 209 and c1 <= 1013 then c1 = c1 + 10
3040 poke 65294, c1 and 255
3050 poke 65298, (peek(65298) and 252) or (int(c1 / 256) and 3)
3060 poke 205,14 : poke 202,0 : sys 55464
3070 print using "####"; c1;
3080 print " "; hex$(c1)
3090 return
3997 rem ***************************
3998 rem * channel 2 pitch control *
3999 rem ***************************
4000 if kn = 75 and c2 > 0 then c2 = c2 - 1
4010 if kn = 79 and c2 < 1023 then c2 = c2 + 1
4020 if kn = 203 and c2 >= 10 then c2 = c2 - 10
4030 if kn = 207 and c2 <= 1013 then c2 = c2 + 10
4040 poke 65295, c2 and 255
4050 poke 65296, int(c2 / 256) and 255
4060 poke 205,14 : poke 202,12 : sys 55464
4070 print using "####"; c2;
4080 print " "; hex$(c2)
4090 return
4997 rem *************************
4998 rem * noise / sound control *
4999 rem *************************
5000 n = 1 - n
5010 poke 205,20 : poke 202,12 : sys 55464
5020 if n = 0 then print "sound" : else print "noise"
5030 if ((cv and 32) = 0) and ((cv and 64) = 0) then return
5040 cv = peek(65297)
5050 if n = 0 then poke 65297, (cv and 191) or 32
5060 if n = 1 then poke 65297, (cv and 223) or 64
5070 return
5997 rem ******************
5998 rem * volume control *
5999 rem ******************
6000 v = kn - 48
6010 poke 65297, (peek(65297) and 240) or v
6020 poke 205,14 : poke 202,24 : sys 55464
6030 print using "#"; v
6040 return
6997 rem ********************
6998 rem * toggle channel 1 *
6999 rem ********************
7000 cv = peek(65297)
7010 if (cv and 16) <> 0 then b4 = 1 : else b4 = 0
7020 if b4 = 1 then cv = cv and 239 : else cv = cv or 16
7030 poke 65297, cv
7040 if b4 = 1 then m$ = "off" : else m$ = "on "
7050 poke 205,17 : poke 202,0 : sys 55464
7060 print m$
7070 return
7997 rem ********************
7998 rem * toggle channel 2 *
7999 rem ********************
8000 cv = peek(65297)
8010 if ((cv and 32) <> 0) or ((cv and 64) <> 0) then b5 = 1 : else b5 = 0
8020 if b5 = 1 then cv = cv and 159 : goto 8050
8030 if n = 0 then cv = (cv and 191) or 32
8040 if n = 1 then cv = (cv and 223) or 64
8050 poke 65297, cv
8060 if b5 = 1 then m$ = "off" : else m$ = "on "
8070 poke 205,17 : poke 202,12 : sys 55464
8080 print m$
8090 return
8997 rem ****************
8998 rem * manual input *
8999 rem ****************
9000 poke 205,22 : poke 202,0 : sys 55464
9010 input "pitch 1"; p1$
9020 poke 205,22 : poke 202,18 : sys 55464
9030 input "pitch 2"; p2$
9040 poke 205,22 : poke 202,0 : sys 55464
9050 print "                                        "
9060 p1 = int(val(p1$))
9070 p2 = int(val(p2$))
9080 if p1 > 0 and p1 < 1024 then c1 = p1
9090 if p2 > 0 and p2 < 1024 then c2 = p2
9100 poke 205,14 : poke 202,0 : sys 55464
9110 print using "####"; c1;
9120 print " "; hex$(c1); "   ";
9130 print using "####"; c2;
9140 print " "; hex$(c2); "   ";
9150 poke 65294, c1 and 255
9160 poke 65298, (peek(65298) and 252) or (int(c1 / 256) and 3)
9170 poke 65295, c2 and 255
9180 poke 65296, int(c2 / 256) and 255
9190 return
