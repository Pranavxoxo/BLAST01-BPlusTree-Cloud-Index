#!/usr/bin/env python3
"""Generates the BLAST 01 diagrams as standalone SVG files in docs/diagrams/.

The tree shapes drawn here are copied from the real program output in
docs/sample-output.txt (order 4, sample data set), so the diagrams and the
running application always agree.

Usage:  python3 tools/make_diagrams.py
"""

import os

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "docs", "diagrams")
os.makedirs(OUT, exist_ok=True)

INK = "#1B2430"
MUTED = "#5B6B7C"
RULE = "#C3CEDA"
PAPER = "#FFFFFF"
INT_FILL, INT_STROKE = "#E0F2FE", "#0369A1"     # internal (routing) nodes
LEAF_FILL, LEAF_STROKE = "#DCFCE7", "#15803D"   # leaf (data) nodes
PATH = "#F59E0B"                                 # search path highlight
RANGE = "#7C3AED"                                # range-scan highlight
NEW = "#0F766E"                                  # created / promoted
BAD = "#B91C1C"                                  # removed / overflow
GHOST = "#F1F5F9"

FONT = "'Helvetica Neue', Helvetica, Arial, sans-serif"
MONO = "Menlo, Consolas, 'Courier New', monospace"


def esc(t):
    return str(t).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def head(w, h, title, subtitle=None):
    s = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" width="{w}" height="{h}"
     font-family="{FONT}" role="img" aria-label="{esc(title)}">
  <defs>
    <marker id="arrow" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7"
            orient="auto-start-reverse">
      <path d="M 0 0 L 10 5 L 0 10 z" fill="context-stroke"/>
    </marker>
    <marker id="arrowInk" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7"
            orient="auto-start-reverse"><path d="M 0 0 L 10 5 L 0 10 z" fill="{INK}"/></marker>
    <marker id="arrowLeaf" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="6" markerHeight="6"
            orient="auto"><path d="M 0 0 L 10 5 L 0 10 z" fill="{LEAF_STROKE}"/></marker>
    <marker id="arrowRange" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="6" markerHeight="6"
            orient="auto"><path d="M 0 0 L 10 5 L 0 10 z" fill="{RANGE}"/></marker>
    <marker id="arrowPath" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7"
            orient="auto"><path d="M 0 0 L 10 5 L 0 10 z" fill="{PATH}"/></marker>
    <marker id="diamond" viewBox="0 0 14 10" refX="1" refY="5" markerWidth="14" markerHeight="10"
            orient="auto-start-reverse"><path d="M 1 5 L 7 1 L 13 5 L 7 9 z" fill="{INK}"/></marker>
    <marker id="openArrow" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="9" markerHeight="9"
            orient="auto"><path d="M 0 0 L 10 5 L 0 10" fill="none" stroke="{INK}" stroke-width="1.4"/></marker>
    <marker id="triangle" viewBox="0 0 12 12" refX="11" refY="6" markerWidth="12" markerHeight="12"
            orient="auto"><path d="M 0 0 L 12 6 L 0 12 z" fill="{PAPER}" stroke="{INK}" stroke-width="1.3"/></marker>
  </defs>
  <rect width="{w}" height="{h}" fill="{PAPER}"/>
  <text x="34" y="42" font-size="23" font-weight="600" fill="{INK}">{esc(title)}</text>
'''
    if subtitle:
        s += f'  <text x="34" y="66" font-size="14.5" fill="{MUTED}">{esc(subtitle)}</text>\n'
    return s


def text(x, y, t, size=14, fill=INK, anchor="start", weight="normal", family=None, italic=False):
    fam = f' font-family="{family}"' if family else ""
    sty = ' font-style="italic"' if italic else ""
    return (f'  <text x="{x:.1f}" y="{y:.1f}" font-size="{size}" fill="{fill}" text-anchor="{anchor}" '
            f'font-weight="{weight}"{fam}{sty}>{esc(t)}</text>\n')


def line(x1, y1, x2, y2, stroke=INK, width=1.4, marker=None, dash=None, start_marker=None):
    m = f' marker-end="url(#{marker})"' if marker else ""
    sm = f' marker-start="url(#{start_marker})"' if start_marker else ""
    d = f' stroke-dasharray="{dash}"' if dash else ""
    return (f'  <line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" stroke="{stroke}" '
            f'stroke-width="{width}"{m}{sm}{d}/>\n')


def rect(x, y, w, h, fill, stroke, sw=1.5, rx=6, dash=None):
    d = f' stroke-dasharray="{dash}"' if dash else ""
    return (f'  <rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" rx="{rx}" fill="{fill}" '
            f'stroke="{stroke}" stroke-width="{sw}"{d}/>\n')


def save(name, svg):
    with open(os.path.join(OUT, name), "w") as f:
        f.write(svg + "</svg>\n")
    print("wrote", name)


# ---------------------------------------------------------------------------
# Generic B+ tree drawing
# ---------------------------------------------------------------------------

CELL_W, CELL_H = 46, 32


def node_box(x, y, keys, leaf, stroke=None, fill=None, sw=1.6, key_colors=None):
    """Draws one node as a row of key cells. Returns svg string."""
    w = max(1, len(keys)) * CELL_W
    f = fill or (LEAF_FILL if leaf else INT_FILL)
    st = stroke or (LEAF_STROKE if leaf else INT_STROKE)
    s = rect(x, y, w, CELL_H, f, st, sw, rx=5)
    for i, k in enumerate(keys):
        if i:
            s += line(x + i * CELL_W, y, x + i * CELL_W, y + CELL_H, st, 1)
        col = (key_colors or {}).get(k, INK)
        s += text(x + i * CELL_W + CELL_W / 2, y + 21, k, 14, col, "middle", "600", MONO)
    if not keys:
        s += text(x + CELL_W / 2, y + 21, "( )", 13, BAD, "middle", "600", MONO)
    return s


def n_children(keys):
    return 0 if keys is None else len(keys) + 1


def width_of(keys):
    return CELL_W if keys is None else max(1, len(keys)) * CELL_W


def layout(levels, x0, width, y0, vgap, leaf_gap_min=14):
    """levels: list of levels, each a list of key lists. Children are assigned
    left-to-right (an internal node with k keys owns the next k + 1 nodes below).
    A node may be None: an elided subtree, drawn as a small "..." box that owns
    no children. Returns positions[level][i] = (x, y, w)."""
    leaves = levels[-1]
    widths = [width_of(k) for k in leaves]
    gap = max(leaf_gap_min, (width - sum(widths)) / max(1, len(leaves) - 1))
    gap = min(gap, 60)
    total = sum(widths) + gap * (len(leaves) - 1)
    x = x0 + (width - total) / 2
    pos = [None] * len(levels)
    y_leaf = y0 + vgap * (len(levels) - 1)
    pos[-1] = []
    for w in widths:
        pos[-1].append((x, y_leaf, w))
        x += w + gap
    for li in range(len(levels) - 2, -1, -1):
        row = [None] * len(levels[li])
        child = 0
        for ni, keys in enumerate(levels[li]):
            n = n_children(keys)
            if n:
                first, last = pos[li + 1][child], pos[li + 1][child + n - 1]
                cx = (first[0] + last[0] + last[2]) / 2
                w = width_of(keys)
                row[ni] = (cx - w / 2, y0 + vgap * li, w)
            child += n
        # place elided nodes beside their positioned neighbours
        for ni in range(len(row)):
            if row[ni] is None and ni > 0 and row[ni - 1] is not None:
                px, py, pw = row[ni - 1]
                row[ni] = (px + pw + 24, y0 + vgap * li, CELL_W)
        for ni in range(len(row) - 1, -1, -1):
            if row[ni] is None:
                nx = row[ni + 1][0]
                row[ni] = (nx - 24 - CELL_W, y0 + vgap * li, CELL_W)
        pos[li] = row
    return pos


def draw_tree(levels, pos, path_nodes=(), range_leaves=(), chain=True, edge_color=MUTED,
              node_style=None, key_colors=None):
    """path_nodes: set of (level, index) to highlight as a search path."""
    s = ""
    node_style = node_style or {}
    # edges
    for li in range(len(levels) - 1):
        child = 0
        for ni, keys in enumerate(levels[li]):
            if keys is None:
                continue
            x, y, w = pos[li][ni]
            n = n_children(keys)
            for c in range(n):
                cx, cy, cw = pos[li + 1][child + c]
                px = x + c * CELL_W if keys else x + w / 2
                on_path = (li, ni) in path_nodes and (li + 1, child + c) in path_nodes
                ghost = levels[li + 1][child + c] is None
                col = PATH if on_path else edge_color
                sw = 3 if on_path else 1.2
                s += f'  <circle cx="{px:.1f}" cy="{y + CELL_H:.1f}" r="2.6" fill="{col}"/>\n'
                s += line(px, y + CELL_H, cx + cw / 2, cy, col, sw, dash="4 3" if ghost else None)
            child += n
    # leaf chain
    if chain:
        leaves = pos[-1]
        for i in range(len(leaves) - 1):
            if levels[-1][i] is None or levels[-1][i + 1] is None:
                continue
            x, y, w = leaves[i]
            nx = leaves[i + 1][0]
            in_range = i in range_leaves and (i + 1) in range_leaves
            col = RANGE if in_range else LEAF_STROKE
            s += line(x + w + 1, y + CELL_H / 2, nx - 2, y + CELL_H / 2, col,
                      2.4 if in_range else 1.3, "arrowRange" if in_range else "arrowLeaf")
    # nodes
    for li, lvl in enumerate(levels):
        leaf = li == len(levels) - 1
        for ni, keys in enumerate(lvl):
            x, y, w = pos[li][ni]
            if keys is None:
                s += rect(x, y, w, CELL_H, PAPER, RULE, 1.3, 5, dash="4 3")
                s += text(x + w / 2, y + 20, "…", 15, MUTED, "middle", "700")
                continue
            stroke, fill, sw = None, None, 1.6
            if (li, ni) in path_nodes:
                stroke, sw = PATH, 3.2
            if leaf and ni in range_leaves:
                stroke, sw = (RANGE, 3) if (li, ni) not in path_nodes else (PATH, 3.2)
            if (li, ni) in node_style:
                stroke, fill, sw = node_style[(li, ni)]
            s += node_box(x, y, keys, leaf, stroke, fill, sw, key_colors)
    return s


def legend(x, y, items):
    s = ""
    for i, (kind, label) in enumerate(items):
        yy = y + i * 24
        if kind == "internal":
            s += rect(x, yy - 12, 22, 16, INT_FILL, INT_STROKE, 1.4, 3)
        elif kind == "leaf":
            s += rect(x, yy - 12, 22, 16, LEAF_FILL, LEAF_STROKE, 1.4, 3)
        elif kind == "path":
            s += line(x, yy - 4, x + 22, yy - 4, PATH, 3.2)
        elif kind == "range":
            s += line(x, yy - 4, x + 22, yy - 4, RANGE, 2.6, "arrowRange")
        elif kind == "chain":
            s += line(x, yy - 4, x + 22, yy - 4, LEAF_STROKE, 1.4, "arrowLeaf")
        s += text(x + 32, yy, label, 13.5, INK)
    return s


# Real tree after "Load sample cloud records" (order 4) — from sample-output.txt
SAMPLE_TREE = [
    [[1034, 1058]],
    [[1012, 1021], [1040, 1047], [1066, 1075, 1090]],
    [[1003, 1005, 1008], [1012, 1015, 1019], [1021, 1026, 1029], [1034, 1038], [1040, 1044],
     [1047, 1052], [1058, 1063], [1066, 1070], [1075, 1081, 1086], [1090, 1097]],
]


# ---------------------------------------------------------------------------
# 1. Structure
# ---------------------------------------------------------------------------

def diagram_structure():
    W, H = 1500, 560
    s = head(W, H, "B+ tree of order 4 built from the 24 sample cloud records",
             "Internal nodes hold routing keys only. Every record lives in a leaf, and leaves are linked left to right.")
    pos = layout(SAMPLE_TREE, 40, W - 80, 120, 120)
    s += draw_tree(SAMPLE_TREE, pos)

    # record stubs under each leaf key
    for (x, y, w), keys in zip(pos[-1], SAMPLE_TREE[-1]):
        for i, k in enumerate(keys):
            cx = x + i * CELL_W + CELL_W / 2
            s += line(cx, y + CELL_H, cx, y + CELL_H + 22, MUTED, 1, "arrowInk")
            s += rect(cx - 15, y + CELL_H + 24, 30, 22, GHOST, RULE, 1, 3)
            s += text(cx, y + CELL_H + 39, "rec", 10.5, MUTED, "middle")

    # level labels
    for li, label in enumerate(["Level 0 · root", "Level 1 · internal", "Level 2 · leaves"]):
        s += text(40, pos[li][0][1] - 10, label, 13, MUTED, weight="600").replace(
            "<text ", '<text stroke="#FFFFFF" stroke-width="5" paint-order="stroke" ', 1)

    y = 470
    s += legend(40, y, [("internal", "Internal node: separator keys + child pointers (a signpost, no data)"),
                        ("leaf", "Leaf node: keys + full CloudRecord for each key"),
                        ("chain", "next-pointer linking leaves in key order")])
    s += text(800, y, "Order m = 4  →  at most 3 keys / 4 children per node", 14, INK, weight="600")
    s += text(800, y + 24, "Every non-root node is at least half full (leaf ≥ 2 keys, internal ≥ 1 key)", 13.5, MUTED)
    s += text(800, y + 48, "All leaves are on the same level, so every lookup reads exactly 3 nodes", 13.5, MUTED)
    save("01-bplus-tree-structure.svg", s)


# ---------------------------------------------------------------------------
# 2. Search path + range scan
# ---------------------------------------------------------------------------

def diagram_search_range():
    W, H = 1500, 600
    s = head(W, H, "Point search for 1052 and range search for IDs 1020 – 1050",
             "Point search: one root-to-leaf descent. Range search: one descent to the start, then walk sideways along the leaf chain.")
    pos = layout(SAMPLE_TREE, 40, W - 80, 130, 125)
    path = {(0, 0), (1, 1), (2, 5)}
    rng = {1, 2, 3, 4, 5}
    s += draw_tree(SAMPLE_TREE, pos, path_nodes=path, range_leaves=rng)

    # annotate the path decisions
    rx, ry, rw = pos[0][0]
    s += text(rx + rw + 18, ry + 14, "1034 ≤ 1052 < 1058 → follow pointer 1", 13.5, PATH, weight="600")
    ix, iy, iw = pos[1][1]
    s += text(ix + iw + 18, iy + 14, "1052 ≥ 1047 → follow pointer 2", 13.5, PATH, weight="600")
    lx, ly, lw = pos[2][5]
    s += text(lx + lw / 2, ly + CELL_H + 26, "found 1052", 13.5, PATH, "middle", "600")

    # range bracket under leaves 1..5
    x1 = pos[2][1][0]
    x2 = pos[2][5][0] + pos[2][5][2]
    by = pos[2][0][1] + CELL_H + 52
    s += line(x1, by, x2, by, RANGE, 2)
    s += line(x1, by - 8, x1, by + 8, RANGE, 2)
    s += line(x2, by - 8, x2, by + 8, RANGE, 2)
    s += text((x1 + x2) / 2, by + 24, "range 1020 – 1050: descend to the leaf where 1020 would be, then follow next-pointers",
              13.5, RANGE, "middle", "600")
    s += text((x1 + x2) / 2, by + 44, "until a key > 1050 appears  →  8 records, 3 node reads + 5 leaves walked",
              13.5, RANGE, "middle")

    s += legend(40, 540, [("path", "Search path for key 1052: 3 node reads, 6 key comparisons"),
                          ("range", "Leaves walked by the range scan")])
    save("02-search-and-range-scan.svg", s)


# ---------------------------------------------------------------------------
# 3. Insert with leaf split + internal split
# ---------------------------------------------------------------------------

def diagram_insert_split():
    W, H = 1500, 760
    s = head(W, H, "Inserting record 1078: a leaf split that cascades into an internal split",
             "Real sequence from the demo run (order 4, max 3 keys per node). Only the right-hand part of the tree is shown.")

    col_w = W / 3
    titles = ["① Before: leaf [1075 1081 1086] is full",
              "② Leaf overflows → split, copy 1081 up",
              "③ Parent overflows → split, move 1081 up"]
    notes = [
        ["1078 belongs between 1075 and 1081,", "but that leaf already holds 3 keys."],
        ["[1075 1078 1081 1086] splits into", "[1075 1078] + [1081 1086].",
         "1081 is COPIED up — it stays in the", "leaf because leaves hold the data."],
        ["Parent [1066 1075 1081 1090] now has 4 keys.", "It splits into [1066 1075] + [1090].",
         "1081 is MOVED up into the root —", "internal keys are only signposts.", "Height is unchanged (still 3)."],
    ]
    for c in range(3):
        x0 = c * col_w
        s += text(x0 + 34, 112, titles[c], 16, INK, weight="600")
        if c:
            s += line(x0 + 4, 95, x0 + 4, 640, RULE, 1, dash="4 4")

    leaves_before = [[1058, 1063], [1066, 1070], [1075, 1081, 1086], [1090, 1097]]
    leaves_after = [[1058, 1063], [1066, 1070], [1075, 1078], [1081, 1086], [1090, 1097]]

    # Stage 1
    t1 = [[[1034, 1058]], [None, None, [1066, 1075, 1090]], leaves_before]
    p1 = layout(t1, 30, col_w - 50, 170, 140, 10)
    s += draw_tree(t1, p1, node_style={(2, 2): (BAD, LEAF_FILL, 3)})
    nx, ny, nw = p1[2][2]
    s += rect(nx + nw / 2 - 30, ny + 64, 60, 30, "#FEF3C7", PATH, 1.6, 5)
    s += text(nx + nw / 2, ny + 84, "1078", 14, INK, "middle", "700", MONO)
    s += line(nx + nw / 2, ny + 62, nx + nw / 2, ny + CELL_H + 4, PATH, 2, "arrowPath")
    s += text(nx + nw / 2 + 36, ny + 84, "insert", 12.5, PATH, weight="600")

    # Stage 2
    t2 = [[[1034, 1058]], [None, None, [1066, 1075, 1081, 1090]], leaves_after]
    p2 = layout(t2, col_w + 20, col_w - 40, 170, 140, 8)
    s += draw_tree(t2, p2, node_style={(1, 2): (BAD, INT_FILL, 3), (2, 2): (NEW, LEAF_FILL, 3),
                                       (2, 3): (NEW, LEAF_FILL, 3)},
                   key_colors={1081: NEW, 1078: NEW})

    # Stage 3
    t3 = [[[1034, 1058, 1081]], [None, None, [1066, 1075], [1090]], leaves_after]
    p3 = layout(t3, 2 * col_w + 10, col_w - 50, 170, 140, 8)
    s += draw_tree(t3, p3, node_style={(0, 0): (NEW, INT_FILL, 3), (1, 2): (NEW, INT_FILL, 3),
                                       (1, 3): (NEW, INT_FILL, 3)}, key_colors={1081: NEW})

    for c in range(3):
        for i, n in enumerate(notes[c]):
            s += text(c * col_w + 34, 560 + i * 20, n, 13.5, INK if i < 2 else MUTED)

    s += text(34, 720, "A B+ tree only grows taller when the ROOT splits — the new root sits above the old one, "
              "so every leaf stays at the same depth and the tree is always balanced.", 14, INK, weight="600")
    save("03-insert-leaf-and-internal-split.svg", s)


# ---------------------------------------------------------------------------
# 4. Delete with merge + borrow
# ---------------------------------------------------------------------------

def diagram_delete_merge():
    W, H = 1500, 700
    s = head(W, H, "Deleting records: merge when siblings are at minimum, borrow when they can spare a key",
             "Real sequence from the demo run (order 4: leaf minimum 2 keys, internal minimum 1 key).")
    col_w = W / 3
    titles = ["① Delete 1044 → leaf merge", "② Delete 1052 → merge empties parent",
              "③ Parent underflow → borrow (rotate)"]
    for c in range(3):
        s += text(c * col_w + 34, 112, titles[c], 16, INK, weight="600")
        if c:
            s += line(c * col_w + 4, 95, c * col_w + 4, 620, RULE, 1, dash="4 4")

    def arrow_down(cx, y):
        return line(cx, y, cx, y + 26, MUTED, 1.6, "arrowInk")

    def notes(x, y, rows):
        out = ""
        for i, (t, col, w) in enumerate(rows):
            out += text(x, y + i * 20, t, 13.5, col, weight=w)
        return out

    # Column 1
    t1 = [[[1040, 1047]], [[1034, 1038], [1040, 1044], [1047, 1052]]]
    p1 = layout(t1, 30, col_w - 60, 150, 100, 24)
    s += draw_tree(t1, p1, node_style={(1, 1): (BAD, LEAF_FILL, 3)}, key_colors={1044: BAD})
    s += notes(34, 320, [("[1040] is left with 1 key (minimum is 2).", INK, "normal"),
                         ("Both siblings hold exactly 2, so neither", INK, "normal"),
                         ("can lend → MERGE with the left sibling.", INK, "600")])
    s += arrow_down(col_w / 2, 372)
    t1b = [[[1047]], [[1034, 1038, 1040], [1047, 1052]]]
    p1b = layout(t1b, 30, col_w - 60, 420, 100, 40)
    s += draw_tree(t1b, p1b, node_style={(1, 0): (NEW, LEAF_FILL, 3)})
    s += notes(34, 590, [("Separator 1040 removed from the parent.", MUTED, "normal")])

    # Column 2
    x0 = col_w
    t2 = [[[1047]], [[1034, 1040], [1047, 1052]]]
    p2 = layout(t2, x0 + 30, col_w - 60, 150, 100, 50)
    s += draw_tree(t2, p2, node_style={(1, 1): (BAD, LEAF_FILL, 3)}, key_colors={1052: BAD})
    s += notes(x0 + 34, 320, [("(1038 was deleted in between: no change.)", MUTED, "normal"),
                              ("[1047] underflows, sibling at minimum → merge.", INK, "normal"),
                              ("The parent loses its only separator: EMPTY.", BAD, "600")])
    s += arrow_down(x0 + col_w / 2, 372)
    cx = x0 + col_w / 2
    s += node_box(cx - CELL_W / 2, 420, [], False, BAD, INT_FILL, 3)
    s += line(cx, 420 + CELL_H, cx, 520, MUTED, 1.2)
    s += node_box(cx - 1.5 * CELL_W, 520, [1034, 1040, 1047], True, NEW, LEAF_FILL, 3)
    s += notes(x0 + 34, 590, [("An internal node with 0 keys is below minimum", MUTED, "normal"),
                              ("(1), so ITS parent must now repair it.", MUTED, "normal")])

    # Column 3
    x0 = 2 * col_w
    cx = x0 + col_w / 2
    lx, rx = x0 + 70, x0 + col_w - 70 - CELL_W
    s += node_box(cx - CELL_W, 150, [1034, 1058], False, None, None, 1.6, {1034: PATH})
    s += node_box(lx, 250, [1012, 1021], False, PATH, INT_FILL, 3, {1021: PATH})
    s += node_box(rx, 250, [], False, BAD, INT_FILL, 3)
    s += line(cx - CELL_W, 150 + CELL_H, lx + CELL_W, 250, MUTED, 1.2)
    s += line(cx, 150 + CELL_H, rx + CELL_W / 2, 250, MUTED, 1.2)
    s += notes(x0 + 34, 320, [("Left sibling [1012 1021] can spare a key:", INK, "normal"),
                              ("separator 1034 comes down, 1021 goes up,", INK, "normal"),
                              ("and leaf [1021 1026 1029] changes parent.", INK, "600")])
    s += arrow_down(cx, 372)
    s += node_box(cx - CELL_W, 420, [1021, 1058], False, NEW, INT_FILL, 3, {1021: NEW})
    s += node_box(lx + CELL_W / 2, 520, [1012], False, NEW, INT_FILL, 3)
    s += node_box(rx, 520, [1034], False, NEW, INT_FILL, 3, {1034: NEW})
    s += line(cx - CELL_W, 420 + CELL_H, lx + CELL_W, 520, MUTED, 1.2)
    s += line(cx, 420 + CELL_H, rx + CELL_W / 2, 520, MUTED, 1.2)
    s += notes(x0 + 34, 590, [("Every node is legal again; height still 3.", MUTED, "normal")])

    s += text(34, 665, "Repair order used by the program: borrow from left sibling → borrow from right sibling → merge. "
              "If the root ends up empty, its only child becomes the root and the height drops by one.",
              13.5, INK, weight="600")
    save("04-delete-merge-and-borrow.svg", s)


# ---------------------------------------------------------------------------
# 5. B-tree vs B+ tree
# ---------------------------------------------------------------------------

def diagram_btree_vs_bplus():
    W, H = 1400, 560
    s = head(W, H, "B-tree vs B+ tree holding the same seven records",
             "Both are balanced multi-way search trees of order 4. The difference is where the data lives.")
    half = W / 2
    s += text(34, 112, "B-tree", 18, INK, weight="700")
    s += text(half + 34, 112, "B+ tree", 18, INK, weight="700")
    s += line(half, 95, half, 470, RULE, 1, dash="4 4")

    bt = [[[30, 50]], [[10, 20], [40], [60, 70]]]
    pb = layout(bt, 40, half - 80, 160, 140, 40)
    s += draw_tree(bt, pb, chain=False, node_style={(0, 0): ("#9333EA", "#F3E8FF", 1.8)})
    for li, lvl in enumerate(bt):
        for ni, keys in enumerate(lvl):
            x, y, w = pb[li][ni]
            for i, k in enumerate(keys):
                cx = x + i * CELL_W + CELL_W / 2
                s += rect(cx - 13, y + CELL_H + 4, 26, 16, GHOST, RULE, 1, 3)
                s += text(cx, y + CELL_H + 16, "rec", 9.5, MUTED, "middle")

    bp = [[[30, 50]], [[10, 20], [30, 40], [50, 60, 70]]]
    pp = layout(bp, half + 40, half - 80, 160, 140, 40)
    s += draw_tree(bp, pp)
    for (x, y, w), keys in zip(pp[-1], bp[-1]):
        for i, k in enumerate(keys):
            cx = x + i * CELL_W + CELL_W / 2
            s += rect(cx - 13, y + CELL_H + 4, 26, 16, GHOST, RULE, 1, 3)
            s += text(cx, y + CELL_H + 16, "rec", 9.5, MUTED, "middle")

    left = ["Every node stores keys AND records.",
            "Each key appears exactly once.",
            "A search can stop early at an internal node.",
            "Range scan = in-order traversal: climb up and down",
            "the tree between neighbouring keys.",
            "Fewer keys fit per node page (records take space)."]
    right = ["Internal nodes store routing keys only.",
             "Separator keys are copies (30, 50 appear twice).",
             "Every search ends at a leaf — predictable cost.",
             "Range scan = find first leaf, then follow next",
             "pointers. No climbing back up.",
             "More keys per page → higher fan-out → shorter tree."]
    for i, (a, b) in enumerate(zip(left, right)):
        ind = 13 if i == 4 else 0
        s += text(40 + ind, 380 + i * 22, ("" if ind else "• ") + a, 13.5, INK)
        s += text(half + 40 + ind, 380 + i * 22, ("" if ind else "• ") + b, 13.5, INK)
    s += text(34, 530, "This is why relational databases and cloud key-value stores build their indexes as B+ trees.",
              14.5, INK, weight="600")
    save("05-btree-vs-bplus-tree.svg", s)


# ---------------------------------------------------------------------------
# 6. Cloud architecture context
# ---------------------------------------------------------------------------

def box(x, y, w, h, title, lines=(), fill=GHOST, stroke=RULE, title_col=INK, sw=1.5):
    s = rect(x, y, w, h, fill, stroke, sw, 8)
    s += text(x + w / 2, y + 26, title, 15, title_col, "middle", "700")
    for i, l in enumerate(lines):
        s += text(x + w / 2, y + 48 + i * 19, l, 12.5, MUTED, "middle")
    return s


def diagram_cloud_architecture():
    W, H = 1440, 790
    s = head(W, H, "Where the B+ tree sits in a cloud database service",
             "Simplified read path for \"GET record 1052\" in a managed cloud database (e.g. Amazon Aurora, Azure SQL, Cloud SQL).")

    s += box(40, 130, 220, 110, "Client apps", ["web / mobile / services", "send queries over HTTPS"])
    s += box(330, 130, 240, 110, "API / SQL endpoint", ["auth, routing to", "the right partition"])
    s += box(640, 130, 250, 110, "Query engine", ["parses the request,", "chooses: index or full scan?"],
             fill="#FEF3C7", stroke=PATH)
    s += line(260, 185, 326, 185, INK, 1.6, "arrowInk")
    s += line(570, 185, 636, 185, INK, 1.6, "arrowInk")

    # index block
    s += rect(960, 110, 440, 360, "#F8FAFC", INT_STROKE, 2, 10)
    s += text(1180, 138, "B+ tree index on record_id", 16, INT_STROKE, "middle", "700")
    s += node_box(1180 - CELL_W, 160, [1034, 1058], False)
    s += node_box(1020, 240, [1012, 1021], False)
    s += node_box(1180 - CELL_W, 240, [1040, 1047], False, PATH, INT_FILL, 3)
    s += node_box(1250, 240, [1066, 1075], False)
    s += line(1180 - CELL_W, 192, 1020 + CELL_W, 240, MUTED)
    s += line(1180, 192, 1180, 240, PATH, 3)
    s += line(1180 + CELL_W, 192, 1250 + CELL_W, 240, MUTED)
    lx = 990
    for i, keys in enumerate([[1034, 1038], [1040, 1044], [1047, 1052], [1058, 1063]]):
        x = lx + i * 102
        st = (PATH, LEAF_FILL, 3) if keys == [1047, 1052] else (None, None, 1.6)
        s += node_box(x, 330, keys, True, st[0], st[1], st[2])
        if i < 3:
            s += line(x + 2 * CELL_W + 1, 346, x + 100, 346, LEAF_STROKE, 1.3, "arrowLeaf")
    s += line(1180 + CELL_W, 272, 1194, 330, PATH, 3)
    s += text(1180, 395, "root + internal pages: tiny, stay cached in RAM", 12.5, MUTED, "middle")
    s += text(1180, 415, "leaf pages: on SSD / network block storage", 12.5, MUTED, "middle")
    s += text(1180, 445, "1 page read per level → 3–4 reads for millions of rows", 13, INT_STROKE, "middle", "700")
    s += line(890, 185, 956, 185, PATH, 2.4, "arrowPath")
    s += text(923, 175, "index", 12, PATH, "middle", "600")

    # storage
    s += box(40, 520, 850, 100, "Distributed storage layer",
             ["pages replicated across availability zones (e.g. Aurora keeps 6 copies across 3 AZs)",
              "a page = one B+ tree node (16 KB in MySQL InnoDB, 8 KB in PostgreSQL and SQL Server)"])
    s += line(1180, 470, 1180, 570, INK, 1.6)
    s += line(1180, 570, 894, 570, INK, 1.6, "arrowInk")
    s += text(1040, 560, "fetch only the leaf page needed", 12.5, MUTED, "middle")

    s += box(640, 300, 250, 120, "Without an index", ["full table scan:", "read every page,", "cost grows with row count"],
             fill="#FEF2F2", stroke=BAD, title_col=BAD)
    s += line(765, 240, 765, 296, BAD, 1.6, "arrowInk", dash="5 4")
    s += line(765, 420, 765, 516, BAD, 1.6, "arrowInk", dash="5 4")
    s += text(775, 475, "read ALL pages", 12.5, BAD)

    notes = [
        "Why B+ trees fit the cloud:",
        "•  High fan-out (hundreds of keys per page) keeps the tree 3–4 levels deep, so network/disk round-trips per lookup stay small.",
        "•  Linked leaves make range queries (\"all records created by tenant X between IDs a and b\") a sequential read.",
        "•  Splits and merges are local to one path, so many clients can read and write different parts of the index concurrently.",
    ]
    for i, n in enumerate(notes):
        s += text(40, 680 + i * 24, n, 14 if i == 0 else 13.5, INK, weight="700" if i == 0 else "normal")
    save("06-cloud-architecture.svg", s)


# ---------------------------------------------------------------------------
# 7. UML class diagram
# ---------------------------------------------------------------------------

def uml_class(x, y, w, name, attrs, methods, stereo=None, note_col=INT_STROKE):
    lh = 18
    top = 40 if stereo else 30
    h = top + 10 + lh * len(attrs) + 10 + lh * len(methods) + 8
    s = rect(x, y, w, h, PAPER, INK, 1.4, 2)
    s += f'  <rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{top}" fill="{INT_FILL}" stroke="{INK}" stroke-width="1.4"/>\n'
    if stereo:
        s += text(x + w / 2, y + 15, stereo, 11.5, MUTED, "middle", italic=True)
        s += text(x + w / 2, y + 33, name, 15, INK, "middle", "700")
    else:
        s += text(x + w / 2, y + 21, name, 15, INK, "middle", "700")
    yy = y + top + 6
    for a in attrs:
        yy += lh
        s += text(x + 10, yy - 4, a, 12, INK, family=MONO)
    yy += 8
    s += line(x, yy, x + w, yy, INK, 1)
    for m in methods:
        yy += lh
        s += text(x + 10, yy - 2, m, 12, INK, family=MONO)
    return s, h


def diagram_uml():
    W, H = 1660, 1060
    s = head(W, H, "UML class diagram — CloudVault Record Index",
             "+ public   - private   # nested type.  Filled diamond = composition (owner destroys the part).")

    app, h_app = uml_class(40, 100, 430, "CloudIndexApp", [
        "- index_ : unique_ptr<BPlusTree>",
        "- arrivalOrder_ : vector<int>",
    ], [
        "+ CloudIndexApp(order : int = 4)",
        "+ run() : void",
        "- printBanner() / printMenu() : void",
        "- loadSampleData() : void",
        "- addRecord() : void",
        "- searchRecord() : void",
        "- rangeSearch() : void",
        "- updateRecord() : void",
        "- deleteRecord() : void",
        "- showStructure() / showAllRecords() : void",
        "- showStatistics() : void",
        "- compareWithFullScan() : void",
        "- scalabilityExperiment() : void",
        "- rebuildWithNewOrder() : void",
        "- readRecordId(prompt) : int",
        "- readRecordFields(id) : CloudRecord",
    ])

    tree, h_tree = uml_class(600, 100, 520, "BPlusTree", [
        "- root_ : Node*",
        "- order_ : int",
        "- size_ : int",
        "- splits_, merges_, borrows_ : int",
        "- changes_ : vector<string>",
    ], [
        "+ BPlusTree(order : int = 4)",
        "+ insert(rec : const CloudRecord&) : bool",
        "+ search(key : int, stats : SearchStats*) : const CloudRecord*",
        "+ update(rec : const CloudRecord&) : bool",
        "+ remove(key : int) : bool",
        "+ rangeQuery(low, high : int, stats) : vector<CloudRecord>",
        "+ allRecords() : vector<CloudRecord>",
        "+ height() / size() / nodeCount() / leafCount() : int",
        "+ printTree(out) / printLeafChain(out) : void",
        "+ lastChanges() : const vector<string>&",
        "+ validate(error : string*) : bool",
        "- findLeaf(key, stats) : Node*",
        "- insertInto(node, rec, promotedKey&, newSibling&) : bool",
        "- splitLeaf(...) / splitInternal(...) : void",
        "- removeFrom(node, key) : bool",
        "- fixUnderflow(parent, childPos) : void",
        "- borrowFromLeft(...) / borrowFromRight(...) : void",
        "- mergeChildren(parent, leftPos) : void",
    ])

    node, h_node = uml_class(1240, 100, 320, "BPlusTree::Node", [
        "+ leaf : bool",
        "+ keys : vector<int>",
        "+ children : vector<Node*>",
        "+ records : vector<CloudRecord>",
        "+ next : Node*",
    ], ["+ Node(isLeaf : bool)"], stereo="«struct» # nested, private")

    rec, h_rec = uml_class(1240, 420, 320, "CloudRecord", [
        "+ recordId : int  {key}",
        "+ tenant : string",
        "+ resourceType : string",
        "+ region : string",
        "+ sizeGB : double",
        "+ createdDate : string",
    ], ["+ CloudRecord(id, tenant, type,", "      region, size, date)", "+ summary() : string"], stereo="«struct»")

    stats, h_stats = uml_class(600, 640, 320, "SearchStats", [
        "+ nodesVisited : int",
        "+ keyComparisons : int",
        "+ leavesScanned : int",
        "+ path : vector<string>",
    ], [], stereo="«struct»")

    inp, h_inp = uml_class(40, 640, 430, "input", [], [
        "+ isScripted() : bool",
        "+ readLine(prompt) : string",
        "+ readInt(prompt, min, max) : int",
        "+ readDouble(prompt, min, max) : double",
        "+ readName(prompt, maxLen) : string",
        "+ readDate(prompt) : string",
        "+ readChoice(title, options) : int",
        "+ readYesNo(prompt) : bool",
    ], stereo="«namespace» utility functions")

    closed, h_closed = uml_class(40, 935, 220, "InputClosed", [], [], stereo="«exception»")
    s += rect(330, 945, 160, 34, PAPER, INK, 1.2, 2)
    s += text(410, 967, "std::runtime_error", 12.5, INK, "middle", family=MONO)
    s += line(260, 962, 326, 962, INK, 1.3, "triangle")

    s += app + tree + node + rec + stats + inp + closed

    # Composition: App ◆— BPlusTree (1)
    s += line(472, 160, 598, 160, INK, 1.4, start_marker="diamond")
    s += text(486, 152, "1", 12, INK)
    s += text(585, 152, "1", 12, INK, "end")
    s += text(535, 178, "owns", 12, MUTED, "middle", italic=True)

    # Composition: BPlusTree ◆— Node (0..*)
    s += line(1122, 160, 1238, 160, INK, 1.4, start_marker="diamond")
    s += text(1136, 152, "1", 12, INK)
    s += text(1226, 152, "0..*", 12, INK, "end")
    s += text(1180, 178, "root_", 12, MUTED, "middle", italic=True)

    # Node self-association (children / next)
    nx, ny = 1560, 100
    s += f'  <path d="M {nx} {ny + 60} h 26 v 80 h -26" fill="none" stroke="{INK}" stroke-width="1.3" marker-end="url(#openArrow)"/>\n'
    s += text(nx + 32, ny + 104, "children", 11.5, MUTED)
    s += text(nx + 32, ny + 118, "next", 11.5, MUTED)

    # Node ◆— CloudRecord (leaf records)
    s += line(1400, 100 + h_node + 2, 1400, 418, INK, 1.4, start_marker="diamond")
    s += text(1410, 100 + h_node + 22, "1", 12, INK)
    s += text(1410, 412, "0..m-1  (leaf only)", 12, INK)

    # BPlusTree ..> SearchStats
    s += line(760, 100 + h_tree + 2, 760, 636, INK, 1.3, "openArrow", dash="6 4")
    s += text(770, 620, "«uses» fills", 12, MUTED, italic=True)

    # App ..> input
    s += line(255, 100 + h_app + 2, 255, 636, INK, 1.3, "openArrow", dash="6 4")
    s += text(265, 620, "«uses» all console input", 12, MUTED, italic=True)

    # App ..> CloudRecord
    s += f'  <path d="M 472 300 H 560 V 600 H 1180 V 520 H 1236" fill="none" stroke="{INK}" stroke-width="1.3" stroke-dasharray="6 4" marker-end="url(#openArrow)"/>\n'
    s += text(1000, 594, "«uses» builds / displays records", 12, MUTED, italic=True)

    # input ..> InputClosed
    s += line(150, 640 + h_inp + 2, 150, 931, INK, 1.3, "openArrow", dash="6 4")
    s += text(160, 920, "«throws»", 12, MUTED, italic=True)

    # main
    s += uml_class(1000, 860, 260, "main.cpp", [], ["+ main(argc, argv) : int"], stereo="«entry point»")[0]
    s += line(1000, 905, 495, 905, INK, 1.3, dash="6 4")
    s += f'  <path d="M 495 905 V 420 H 474" fill="none" stroke="{INK}" stroke-width="1.3" stroke-dasharray="6 4" marker-end="url(#openArrow)"/>\n'
    s += text(760, 897, "«creates» CloudIndexApp(order) and calls run()", 12, MUTED, "middle", italic=True)

    save("07-uml-class-diagram.svg", s)


# ---------------------------------------------------------------------------
# 8. Activity / flow diagram
# ---------------------------------------------------------------------------

def flow_box(x, y, w, h, label, fill=GHOST, stroke=RULE, bold=False, lines=None):
    s = rect(x, y, w, h, fill, stroke, 1.5, 8)
    ls = lines or [label]
    for i, l in enumerate(ls):
        s += text(x + w / 2, y + h / 2 + 5 + (i - (len(ls) - 1) / 2) * 17, l, 13, INK, "middle",
                  "700" if bold else "normal")
    return s


def diamond(cx, cy, w, h, label_lines, stroke=PATH):
    s = (f'  <path d="M {cx} {cy - h / 2} L {cx + w / 2} {cy} L {cx} {cy + h / 2} L {cx - w / 2} {cy} Z" '
         f'fill="#FEF3C7" stroke="{stroke}" stroke-width="1.6"/>\n')
    for i, l in enumerate(label_lines):
        s += text(cx, cy + 5 + (i - (len(label_lines) - 1) / 2) * 16, l, 12.5, INK, "middle")
    return s


def terminal(cx, cy, label):
    return (f'  <rect x="{cx - 70}" y="{cy - 18}" width="140" height="36" rx="18" fill="{INK}"/>\n'
            + text(cx, cy + 5, label, 13.5, PAPER, "middle", "700"))


def diagram_activity():
    W, H = 1500, 1000
    s = head(W, H, "Activity diagrams — menu loop and the search algorithm",
             "Left: how the application runs. Right: what option 3 does inside the B+ tree.")

    # ---- Left: menu loop ----
    cx = 330
    s += terminal(cx, 115, "Start")
    s += line(cx, 133, cx, 158, INK, 1.4, "arrowInk")
    s += flow_box(cx - 150, 160, 300, 46, "", lines=["Create CloudIndexApp(order)", "print banner"])
    s += line(cx, 206, cx, 232, INK, 1.4, "arrowInk")
    s += flow_box(cx - 150, 234, 300, 46, "", lines=["Print menu + status line", "(records, order, height)"])
    s += line(cx, 280, cx, 306, INK, 1.4, "arrowInk")
    s += flow_box(cx - 150, 308, 300, 40, "Read a line from the user")
    s += line(cx, 348, cx, 372, INK, 1.4, "arrowInk")
    s += diamond(cx, 418, 240, 90, ["whole number", "between 0 and 12?"])
    s += line(cx + 120, 418, cx + 200, 418, BAD, 1.4)
    s += line(cx + 200, 418, cx + 200, 328, BAD, 1.4)
    s += line(cx + 200, 328, cx + 152, 328, BAD, 1.4, "arrowInk")
    s += text(cx + 128, 438, "no: print error,", 12, BAD)
    s += text(cx + 128, 453, "ask again", 12, BAD)
    s += line(cx, 463, cx, 492, INK, 1.4, "arrowInk")
    s += text(cx + 8, 482, "yes", 12, NEW)
    s += diamond(cx, 532, 200, 76, ["choice == 0 ?"])
    s += line(cx - 100, 532, cx - 200, 532, INK, 1.4)
    s += line(cx - 200, 532, cx - 200, 900, INK, 1.4)
    s += line(cx - 200, 900, cx - 72, 900, INK, 1.4, "arrowInk")
    s += text(cx - 196, 522, "yes", 12, NEW)
    s += line(cx, 570, cx, 598, INK, 1.4, "arrowInk")
    s += text(cx + 8, 590, "no", 12, MUTED)
    s += flow_box(cx - 160, 600, 320, 150, "", fill="#F8FAFC", stroke=INT_STROKE, lines=[
        "Dispatch to the handler:",
        "1 load sample   2 add   3 search",
        "4 range   5 update   6 delete",
        "7 structure   8 all records   9 stats",
        "10 index vs scan   11 scalability",
        "12 rebuild with new order"])
    s += line(cx, 750, cx, 776, INK, 1.4, "arrowInk")
    s += flow_box(cx - 150, 778, 300, 46, "", lines=["Handler validates its own input,", "prints result + structural changes"])
    s += line(cx + 150, 801, cx + 240, 801, INK, 1.4)
    s += line(cx + 240, 801, cx + 240, 257, INK, 1.4)
    s += line(cx + 240, 257, cx + 152, 257, INK, 1.4, "arrowInk")
    s += text(cx + 246, 640, "loop", 12, MUTED)
    s += terminal(cx, 900, "Exit")
    s += text(cx - 120, 950, "End of input (Ctrl-D / end of piped file) also exits cleanly via InputClosed.", 12, MUTED)

    # ---- Right: search algorithm ----
    rx = 1060
    s += line(675, 95, 675, 960, RULE, 1, dash="4 4")
    s += terminal(rx, 115, "search(key)")
    s += line(rx, 133, rx, 158, INK, 1.4, "arrowInk")
    s += diamond(rx, 196, 200, 70, ["tree empty?"])
    s += line(rx + 100, 196, rx + 250, 196, INK, 1.4)
    s += line(rx + 250, 196, rx + 290, 196, INK, 1.4)
    s += line(rx + 290, 196, rx + 290, 785, INK, 1.4)
    s += line(rx + 290, 785, rx + 254, 785, INK, 1.4, "arrowInk")
    s += text(rx + 108, 188, "yes", 12, NEW)
    s += line(rx, 231, rx, 258, INK, 1.4, "arrowInk")
    s += text(rx + 8, 250, "no", 12, MUTED)
    s += flow_box(rx - 140, 260, 280, 40, "node ← root   (count 1 node read)")
    s += line(rx, 300, rx, 326, INK, 1.4, "arrowInk")
    s += diamond(rx, 366, 200, 76, ["node is a leaf?"])
    s += line(rx - 100, 366, rx - 230, 366, INK, 1.4)
    s += text(rx - 120, 356, "no", 12, MUTED, "end")
    s += flow_box(rx - 330, 400, 260, 64, "", fill="#F8FAFC", stroke=INT_STROKE, lines=[
        "i ← number of separator", "keys ≤ key  (comparisons)", ""])
    s += line(rx - 230, 366, rx - 230, 398, INK, 1.4, "arrowInk")
    s += line(rx - 200, 464, rx - 200, 490, INK, 1.4, "arrowInk")
    s += flow_box(rx - 330, 492, 260, 46, "", fill="#F8FAFC", stroke=INT_STROKE,
                  lines=["node ← children[i]", "(1 more node read)"])
    s += line(rx - 330, 515, rx - 350, 515, INK, 1.4)
    s += line(rx - 350, 515, rx - 350, 314, INK, 1.4)
    s += line(rx - 350, 314, rx - 4, 314, INK, 1.4, "arrowInk")
    s += text(rx - 344, 470, "repeat", 12, MUTED)
    s += line(rx, 404, rx, 580, INK, 1.4, "arrowInk")
    s += text(rx + 8, 430, "yes", 12, NEW)
    s += flow_box(rx - 140, 582, 280, 46, "", lines=["scan leaf keys left to right", "(stop early once key passed)"])
    s += line(rx, 628, rx, 656, INK, 1.4, "arrowInk")
    s += diamond(rx, 696, 200, 76, ["key found?"])
    s += line(rx + 100, 696, rx + 170, 696, INK, 1.4)
    s += line(rx + 170, 696, rx + 170, 760, INK, 1.4, "arrowInk")
    s += text(rx + 108, 688, "no", 12, MUTED)
    s += flow_box(rx + 90, 762, 160, 46, "", fill="#FEF2F2", stroke=BAD, lines=["return nullptr", "→ \"NOT FOUND\""])
    s += line(rx, 734, rx, 760, INK, 1.4, "arrowInk")
    s += text(rx + 8, 752, "yes", 12, NEW)
    s += flow_box(rx - 150, 762, 220, 46, "", fill=LEAF_FILL, stroke=LEAF_STROKE,
                  lines=["return &records[i]", "→ print record + cost"])
    s += text(rx - 300, 930, "Cost = height node reads, independent of where the key sits — every search ends at a leaf.",
              13, INK, weight="600")
    save("08-activity-diagram.svg", s)


if __name__ == "__main__":
    diagram_structure()
    diagram_search_range()
    diagram_insert_split()
    diagram_delete_merge()
    diagram_btree_vs_bplus()
    diagram_cloud_architecture()
    diagram_uml()
    diagram_activity()
