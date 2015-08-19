def MakeRow(byte):
    byte -= 1
    lookups = ["0x0"]

    base = (0xFFFFFFFFFFFFFFFF >> ((8 - byte) * 8)) + 1
    cap = (0xFF << (byte * 8)) + 1

    for i in range(base, cap, base):
        string = "0x"
        n = str(i)

        for x in n:
            string += "0" + x

        lookups.append(string)

    return lookups

def SplitLowHigh(lookups):
    lookups_low = []
    lookups_high = []

    for l in lookups:
        high = l[:-8]
        l_h = len(high)
        if l_h <= 2:
            high = "0x0"
            if l_h == 0:
                low = l[-8:]
            elif l_h == 1:
                low = "0" + l[-8:]
            else:
                low = "0x" + l[-8:]
        else:
            low = "0x" + l[-8:]

        lookups_high.append(high)
        lookups_low.append(low)

    return (lookups_high, lookups_low)

def MakeSplitRow(byte):
    lookups = MakeRow(byte)
    return SplitLowHigh(lookups)

def MakeSplitRow3(byte):
    lookups_highmid_low = MakeSplitRow(byte)
    lookups_high_mid = SplitLowHigh(lookups_highmid_low[0])
    return (lookups_high_mid[0], lookups_high_mid[1], lookups_highmid_low[1])

def MakeSplitRow4(byte):
    lookups_high_mid_low = MakeSplitRow3(byte)
    lookups_high_midhigh = SplitLowHigh(lookups_high_mid_low[0])
    return (lookups_high_midhigh[0], lookups_high_midhigh[1],
            lookups_high_mid_low[1], lookups_high_mid_low[2])

def MakeSplitRow5(byte):
    lookups_high_midhigh_midlow_low = MakeSplitRow4(byte)
    lookups_high_midhighhigh = SplitLowHigh(lookups_high_midhigh_midlow_low[0])
    return (lookups_high_midhighhigh[0], lookups_high_midhighhigh[1],
            lookups_high_midhigh_midlow_low[1], lookups_high_midhigh_midlow_low[2],
            lookups_high_midhigh_midlow_low[3])

def AppendToResult(row, result, comment, bigendian):
    result += "\n    {\n"
    result += "        //" + comment + " "

    for h,i in zip(row, range(len(r0))):
        if i % 4 == 0:
            result = result[:-1] + "\n        "

        if bigendian:
            result += h
        else:
            rev = h[:1:-1][:-1]
            if len(rev) == 0:
                rev = "0"
            result += "0x" + rev
        result += ", "

    result = result[:-2] + "\n    },"
    return result

#row setup
r0 = MakeRow(1)

r1_2 = MakeSplitRow(2)
r3_4 = MakeSplitRow(3)

r5_6_7 = MakeSplitRow3(4)

r8_9_10_11 = MakeSplitRow4(5)
r12_13_14_15 = MakeSplitRow4(6)

r16_17_18_19_20 = MakeSplitRow5(7)
r21_22_23_24_25 = MakeSplitRow5(8)

def CreateResultTable(bigendian, result):
    result = AppendToResult(r0, result, "byte 1", bigendian)

    result = AppendToResult(r1_2[0], result, "byte 2 high", bigendian)
    result = AppendToResult(r1_2[1], result, "byte 2 low", bigendian)
    result = AppendToResult(r3_4[0], result, "byte 3 high", bigendian)
    result = AppendToResult(r3_4[1], result, "byte 3 low", bigendian)

    result = AppendToResult(r5_6_7[0], result, "byte 4 high", bigendian)
    result = AppendToResult(r5_6_7[1], result, "byte 4 mid", bigendian)
    result = AppendToResult(r5_6_7[2], result, "byte 4 low", bigendian)

    result = AppendToResult(r8_9_10_11[0], result, "byte 5 high", bigendian)
    result = AppendToResult(r8_9_10_11[1], result, "byte 5 mid high", bigendian)
    result = AppendToResult(r8_9_10_11[2], result, "byte 5 mid low", bigendian)
    result = AppendToResult(r8_9_10_11[3], result, "byte 5 low", bigendian)
    result = AppendToResult(r12_13_14_15[0], result, "byte 6 high", bigendian)
    result = AppendToResult(r12_13_14_15[1], result, "byte 6 mid high", bigendian)
    result = AppendToResult(r12_13_14_15[2], result, "byte 6 mid low", bigendian)
    result = AppendToResult(r12_13_14_15[3], result, "byte 6 low", bigendian)

    result = AppendToResult(r16_17_18_19_20[0], result, "byte 7 high", bigendian)
    result = AppendToResult(r16_17_18_19_20[1], result, "byte 7 mid high", bigendian)
    result = AppendToResult(r16_17_18_19_20[2], result, "byte 7 mid mid", bigendian)
    result = AppendToResult(r16_17_18_19_20[3], result, "byte 7 mid low", bigendian)
    result = AppendToResult(r16_17_18_19_20[4], result, "byte 7 low", bigendian)
    result = AppendToResult(r21_22_23_24_25[0], result, "byte 8 high", bigendian)
    result = AppendToResult(r21_22_23_24_25[1], result, "byte 8 mid high", bigendian)
    result = AppendToResult(r21_22_23_24_25[2], result, "byte 8 mid mid", bigendian)
    result = AppendToResult(r21_22_23_24_25[3], result, "byte 8 mid low", bigendian)
    result = AppendToResult(r21_22_23_24_25[4], result, "byte 8 low", bigendian)

    return result


#big-endian table creation
result = "constexpr static const uint32_t i10toa_lut_be[26][256] = {"
result = CreateResultTable(True, result)
result = result[:-1] + "\n};\n\n"
print result

#little-endian table creation
result = "constexpr static const uint32_t i10toa_lut_le[26][256] = {"
result = CreateResultTable(False, result)
result = result[:-1] + "\n};"

print result
