import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Arrow
import matplotlib.patheffects as pe

fig, ax = plt.subplots(figsize=(8, 6))
ax.set_xlim(0, 4); ax.set_ylim(0, 3)
ax.set_aspect('equal'); ax.set_axis_off()

# Board outline
ax.add_patch(Rectangle((0, 0), 4, 3, fill=False, edgecolor='black', lw=2))

# FPGA (center)
ax.add_patch(Rectangle((1.5, 1), 1, 1, fc='blue', alpha=0.5))
ax.text(2, 1.5, 'Zynq-7020\n(Apollo 68080)', ha='center', va='center', color='white')

# HDMI (left)
ax.add_patch(Rectangle((0.1, 2.5), 0.4, 0.4, fc='red', alpha=0.5))
ax.text(0.3, 2.7, 'HDMI TX/RX', ha='center', fontsize=8)

# M.2 (top-right)
ax.add_patch(Rectangle((3.2, 2.2), 0.7, 0.3, fc='green', alpha=0.5))
ax.text(3.55, 2.35, 'M.2 (PCIe x2)', ha='center', fontsize=8)

# 2.5GbE PMOD (bottom-right)
ax.add_patch(Rectangle((3.2, 0.2), 0.5, 0.3, fc='yellow', alpha=0.5))
ax.text(3.45, 0.35, '2.5GbE PMOD', ha='center', fontsize=8)

# DDR3 (near FPGA)
ax.add_patch(Rectangle((2.8, 1.2), 0.6, 0.3, fc='purple', alpha=0.5))
ax.text(3.1, 1.35, 'DDR3', ha='center', fontsize=8)

# GPIO (right)
ax.add_patch(Rectangle((3.8, 0.8), 0.15, 1.2, fc='gray', alpha=0.5))
ax.text(3.9, 1.4, 'GPIO', ha='center', fontsize=8)

# Wide Bus Arrow
ax.add_patch(Arrow(2, 1, 0.8, 0.2, width=0.3, fc='cyan', alpha=0.7, path_effects=[pe.Stroke(linewidth=2, foreground='black')]))
ax.text(2.4, 1.3, '1024-bit Bus', fontsize=8, color='black')

plt.title('PYNQ-Z2 68000 SBC (~4"x3")', fontsize=12)
plt.savefig('sbc_board_layout.png', dpi=300, bbox_inches='tight')
plt.close()
