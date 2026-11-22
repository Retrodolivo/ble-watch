"""Generate atopile atomic parts (footprint + simple symbol + .ato) for the watch."""
import os, re, shutil, sys
FP = sys.argv[1]   # kicad-footprints checkout
SYM = sys.argv[2]  # kicad-symbols checkout
OUT = sys.argv[3]  # project parts dir

def sym_box(name, pins):
    """Minimal KiCad 8 symbol: a box with pins on left (first half) and right."""
    n = len(pins); left = pins[: (n + 1) // 2]; right = pins[(n + 1) // 2:]
    h = max(len(left), len(right)) * 2.54 + 2.54
    w = 15.24
    out = [f'(kicad_symbol_lib (version 20231120) (generator "watch-genparts")',
           f'  (symbol "{name}" (in_bom yes) (on_board yes)',
           f'    (property "Reference" "U" (at 0 {h/2+2.54:.2f} 0) (effects (font (size 1.27 1.27))))',
           f'    (property "Value" "{name}" (at 0 {-h/2-2.54:.2f} 0) (effects (font (size 1.27 1.27))))',
           f'    (symbol "{name}_0_1" (rectangle (start {-w/2} {h/2:.2f}) (end {w/2} {-h/2:.2f}) (stroke (width 0.254) (type default)) (fill (type background))))',
           f'    (symbol "{name}_1_1"']
    for side, lst in ((-1, left), (1, right)):
        for i, (num, pname) in enumerate(lst):
            y = h / 2 - 2.54 * (i + 1)
            x = side * (w / 2 + 2.54)
            ang = 0 if side < 0 else 180
            out.append(f'      (pin passive line (at {x:.2f} {y:.2f} {ang}) (length 2.54) (name "{pname}" (effects (font (size 1.0 1.0)))) (number "{num}" (effects (font (size 1.0 1.0)))))')
    out += ['    )', '  )', ')']
    return "\n".join(out) + "\n"

def ident(s):
    s = s.replace("+", "P").replace("-", "N") if s in ("D+", "D-") else s
    s = re.sub(r"~\{(\w+)\}", r"N\1", s)
    s = s.split("/")[-1] if "/" in s else s  # AIN0/P0.02 -> P0.02
    s = re.sub(r"[^A-Za-z0-9_]", "_", s)
    if s[0].isdigit(): s = "_" + s
    return s

def part(name, fp_path, mpn, mfr, prefix, doc, pins, bridge=None, symbol=None, lcsc="TBD"):
    d = os.path.join(OUT, name); os.makedirs(d, exist_ok=True)
    shutil.copy(os.path.join(FP, fp_path), d)
    fp = os.path.basename(fp_path)
    fpf = os.path.join(d, fp); txt = open(fpf).read()
    if fp == "D_0402_1005Metric.kicad_mod":
        # atopile 0.15 cannot size a footprint whose silkscreen is only a circle: add a short line
        txt = txt.replace("\t(pad ", '\t(fp_line (start -0.3 -0.6) (end 0.3 -0.6) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))\n\t(pad ', 1)
        open(fpf, "w").write(txt)
    if symbol:
        shutil.copy(os.path.join(SYM, symbol), os.path.join(d, name + ".kicad_sym"))
    else:
        open(os.path.join(d, name + ".kicad_sym"), "w").write(sym_box(name, pins))
    lines = ['#pragma experiment("TRAITS")', "import is_atomic_part", "import has_designator_prefix", "import has_part_picked",
             "import can_bridge_by_name" if bridge else "", "",
             f"component {name}:", f'    """{doc}"""',
             f'    trait is_atomic_part<manufacturer="{mfr}", partnumber="{mpn}", footprint="{fp}", symbol="{name}.kicad_sym">',
             f'    trait has_part_picked::by_supplier<supplier_id="lcsc", supplier_partno="{lcsc}", manufacturer="{mfr}", partno="{mpn}">',
             f'    trait has_designator_prefix<prefix="{prefix}">']
    if bridge:
        lines.append(f'    trait can_bridge_by_name<input_name="{bridge[0]}", output_name="{bridge[1]}">')
    lines.append("")
    seen = {}
    for num, pname in pins:
        if pname is None:
            lines.append(f"    pin {num}"); continue
        i = ident(pname)
        if i in seen:
            lines.append(f"    {i} ~ pin {num}")
        else:
            seen[i] = 1; lines.append(f"    signal {i} ~ pin {num}")
    open(os.path.join(d, name + ".ato"), "w").write("\n".join(l for l in lines if l is not None) + "\n")

# ---------- passives (value baked into the part) ----------
R = "Resistor_SMD.pretty/R_0402_1005Metric.kicad_mod"
C4 = "Capacitor_SMD.pretty/C_0402_1005Metric.kicad_mod"
C6 = "Capacitor_SMD.pretty/C_0603_1608Metric.kicad_mod"
L4 = "Inductor_SMD.pretty/L_0402_1005Metric.kicad_mod"
L6 = "Inductor_SMD.pretty/L_0603_1608Metric.kicad_mod"
two = [("1", "p1"), ("2", "p2")]
passives = [
    ("R_1k_0402", R, "RC0402FR-071KL", "YAGEO", "R", "1 kOhm 1% 0402 (LED current limit)", "Device.kicad_symdir/R.kicad_sym", "C106235"),
    ("R_4k7_0402", R, "RC0402FR-074K7L", "YAGEO", "R", "4.7 kOhm 1% 0402 (I2C pull-up, charge current set)", "Device.kicad_symdir/R.kicad_sym", "C105871"),
    ("R_10k_0402", R, "RC0402FR-0710KL", "YAGEO", "R", "10 kOhm 1% 0402 (NTC substitute, status LED current limit)", "Device.kicad_symdir/R.kicad_sym", "C60490"),
    ("R_1M_0402", R, "RC0402FR-071ML", "YAGEO", "R", "1 MOhm 1% 0402 (battery voltage divider)", "Device.kicad_symdir/R.kicad_sym", "C138033"),
    ("R_0R_0402", R, "RC0402JR-070RL", "YAGEO", "R", "0 Ohm jumper 0402 (antenna tuning placeholder)", "Device.kicad_symdir/R.kicad_sym", "C60485"),
    ("C_100nF_0402", C4, "GRM155R71C104KA88D", "Murata", "C", "100 nF 16 V X7R 0402", "Device.kicad_symdir/C.kicad_sym", "C71629"),
    ("C_1uF_0402", C4, "GRM155R61A105KE15D", "Murata", "C", "1 uF 10 V X5R 0402", "Device.kicad_symdir/C.kicad_sym", "C76999"),
    ("C_1uF_25V_0603", C6, "GRM188R61E105KA12D", "Murata", "C", "1 uF 25 V X5R 0603 (VBUS input)", "Device.kicad_symdir/C.kicad_sym", "C77046"),
    ("C_4u7_0603", C6, "GRM188R61A475KE15D", "Murata", "C", "4.7 uF 10 V X5R 0603", "Device.kicad_symdir/C.kicad_sym", "C71633"),
    ("C_10uF_0603", C6, "GRM188R61A106KE69D", "Murata", "C", "10 uF 10 V X5R 0603", "Device.kicad_symdir/C.kicad_sym", "C77044"),
    ("C_12pF_0402", C4, "GJM1555C1H120GB01D", "Murata", "C", "12 pF 50 V C0G 0402 (crystal load)", "Device.kicad_symdir/C.kicad_sym", "C161305"),
    ("C_100pF_0402", C4, "GRM1555C1H101JA01D", "Murata", "C", "100 pF 50 V C0G 0402 (RF decoupling)", "Device.kicad_symdir/C.kicad_sym", "C77177"),
    ("C_0p8pF_0402", C4, "GJM1555C1HR80BB01D", "Murata", "C", "0.8 pF C0G 0402 (RF matching)", "Device.kicad_symdir/C.kicad_sym", "C76915"),
    ("L_10uH_0603", L6, "LQM18DZ100M70L", "Murata", "L", "10 uH 0603 power inductor (nRF52840 DC/DC) - VERIFY part", "Device.kicad_symdir/L.kicad_sym", "C3911082"),
    ("L_15nH_0402", L4, "LQG15HS15NJ02D", "Murata", "L", "15 nH 0402 (nRF52840 DC/DC)", "Device.kicad_symdir/L.kicad_sym", "C86059"),
    ("L_3n9H_0402", L4, "LQG15HH3N9B02D", "Murata", "L", "3.9 nH 0402 (RF matching)", "Device.kicad_symdir/L.kicad_sym", "C907911"),
    ("L_2u2H_DFE201610", "Inductor_SMD.pretty/L_Murata_DFE201610P.kicad_mod", "DFE201610E-2R2M=P2", "Murata", "L", "2.2 uH 2016 power inductor (nPM1100 buck)", "Device.kicad_symdir/L.kicad_sym", "C337891"),
]
for n, fp, mpn, mfr, pre, doc, sym, lcsc in passives:
    part(n, fp, mpn, mfr, pre, doc, two, bridge=("p1", "p2"), symbol=sym, lcsc=lcsc)

# ---------- diodes ----------
part("LED_Yellow_0603", "LED_SMD.pretty/LED_0603_1608Metric.kicad_mod", "LTST-C190KSKT", "Lite-On", "D",
     "Yellow LED 0603. Pad 1 = cathode.", [("1", "K"), ("2", "A")], bridge=("A", "K"), symbol="Device.kicad_symdir/LED.kicad_sym", lcsc="C125095")
part("LED_Red_0603", "LED_SMD.pretty/LED_0603_1608Metric.kicad_mod", "LTST-C190KRKT", "Lite-On", "D",
     "Red LED 0603 (631 nm, AlInGaP). Pad 1 = cathode.", [("1", "K"), ("2", "A")], bridge=("A", "K"), symbol="Device.kicad_symdir/LED.kicad_sym", lcsc="C94869")
part("LED_Green_0603", "LED_SMD.pretty/LED_0603_1608Metric.kicad_mod", "LTST-C190KGKT", "Lite-On", "D",
     "Yellow-green LED 0603 (572 nm, AlInGaP, Vf close to the red one). Pad 1 = cathode.", [("1", "K"), ("2", "A")], bridge=("A", "K"), symbol="Device.kicad_symdir/LED.kicad_sym", lcsc="C125094")
part("D_RB521S30", "Diode_SMD.pretty/D_SOD-523.kicad_mod", "RB521S30T1G", "onsemi", "D",
     "30 V 200 mA Schottky, SOD-523. Pad 1 = cathode.", [("1", "K"), ("2", "A")], bridge=("A", "K"), symbol="Device.kicad_symdir/D_Schottky.kicad_sym", lcsc="C145179")
part("D_TVS_TPD1E10B06", "Diode_SMD.pretty/D_0402_1005Metric.kicad_mod", "TPD1E10B06DPYR", "Texas Instruments", "D",
     "6 V bidirectional ESD/TVS, 0402 (X1SON). VERIFY footprint vs DPY land pattern.", [("1", "IO"), ("2", "GND")], symbol="Device.kicad_symdir/D_TVS.kicad_sym", lcsc="C48260")

# ---------- crystals / antenna ----------
part("Y_32MHz_2016", "Crystal.pretty/Crystal_SMD_2016-4Pin_2.0x1.6mm.kicad_mod", "NX2016SA-32M-EXS00A-CS06465", "NDK", "Y",
     "32 MHz crystal, 2016, CL = 8 pF, +/-10 ppm (Nordic-qualified)", [("1", "X1"), ("2", "GND"), ("3", "X2"), ("4", "GND")], symbol="Device.kicad_symdir/Crystal_GND24.kicad_sym", lcsc="C1986260")
part("Y_32k768_2012", "Crystal.pretty/Crystal_SMD_2012-2Pin_2.0x1.2mm.kicad_mod", "ABS06-32.768KHZ-9-T", "Abracon", "Y",
     "32.768 kHz crystal, 2012, CL = 9 pF", [("1", "X1"), ("2", "X2")], symbol="Device.kicad_symdir/Crystal.kicad_sym", lcsc="C1985410")
part("ANT_2450AT18A100", "RF_Antenna.pretty/Johanson_2450AT18x100.kicad_mod", "2450AT18A100E", "Johanson Technology", "AE",
     "2.45 GHz chip antenna. Pad 1 = feed, pad 2 = mechanical (no connect).", [("1", "FEED"), ("2", "NC")], symbol="Device.kicad_symdir/Antenna_Chip.kicad_sym", lcsc="C89334")

# ---------- connectors / pads ----------
part("TC2030_NL", "Connector.pretty/Tag-Connect_TC2030-IDC-NL_2x03_P1.27mm_Vertical.kicad_mod", "TC2030-IDC-NL", "Tag-Connect", "J",
     "Tag-Connect footprint (no part placed in production). ARM SWD pinout.",
     [("1", "VTREF"), ("2", "SWDIO"), ("3", "NRESET"), ("4", "SWDCLK"), ("5", "GND"), ("6", "SWO")], lcsc="none")
part("Pad_2mm", "TestPoint.pretty/TestPoint_Pad_2.0x2.0mm.kicad_mod", "PAD-2x2", "none", "TP",
     "2 x 2 mm bare copper pad (pogo contact / battery tab)", [("1", "PAD")], symbol="Connector.kicad_symdir/TestPoint.kicad_sym", lcsc="none")

# ---------- ICs ----------
part("LIS2DW12TR", "Package_LGA.pretty/LGA-12_2x2mm_P0.5mm.kicad_mod", "LIS2DW12TR", "STMicroelectronics", "U",
     "3-axis accelerometer, LGA-12 2x2x0.7 mm",
     [("1", "SCL"), ("2", "CS"), ("3", "SDO_SA0"), ("4", "SDA"), ("5", "NC"), ("6", "GND"), ("7", "RES"),
      ("8", "GND"), ("9", "VDD"), ("10", "VDD_IO"), ("11", "INT2"), ("12", "INT1")], lcsc="C189624")
part("nPM1100_QDAA", "Package_DFN_QFN.pretty/QFN-24-1EP_4x4mm_P0.5mm_EP2.6x2.6mm.kicad_mod", "NPM1100-QDAA-R7", "Nordic Semiconductor", "U",
     "PMIC: Li-ion charger + 1.8-3.0 V buck, QFN24 4x4. Pinout from LCSC/Flux data; VERIFY with nPM1100 PS (QFN) and EP size.",
     [("1", "VOUTB"), ("2", "VOUTBSET1"), ("3", "VOUTBSET0"), ("4", "AVSS"), ("5", "ISET"), ("6", "SHPACT"),
      ("7", "MODE"), ("8", "CHG"), ("9", "VTERMSET"), ("10", "ERR"), ("11", "SHPHLD"), ("12", "ICHG"),
      ("13", "NC13"), ("14", "NTC"), ("15", "VBAT"), ("16", "VSYS"), ("17", "VBUS"), ("18", "NC18"),
      ("19", "D-"), ("20", "D+"), ("21", "DEC"), ("22", "SW"), ("23", "PVSS"), ("24", "NC24"), ("25", "AVSS")], lcsc="C7249859")

# nRF52840: pins straight from the KiCad symbol (matches the footprint pad names)
s = open(os.path.join(SYM, "MCU_Nordic.kicad_symdir/nRF52840.kicad_sym")).read()
pins = re.findall(r'\(pin \w+ \w+.*?\(name "([^"]*)".*?\(number "([^"]*)"', s, re.S)
pins = sorted({(num, nm) for nm, num in pins})
part("nRF52840_QIAA", "Package_DFN_QFN.pretty/Nordic_AQFN-73-1EP_7x7mm_P0.5mm.kicad_mod", "NRF52840-QIAA-R", "Nordic Semiconductor", "U",
     "nRF52840 BLE SoC, aQFN73 7x7 mm", pins, symbol="MCU_Nordic.kicad_symdir/nRF52840.kicad_sym", lcsc="C190794")
print("ok")
