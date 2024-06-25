###############################################################################
# This file is part of MPE Emulator.
# Copyright (C) 2023, 2024  Attila M. Magyar
#
# MPE Emulator is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# MPE Emulator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
###############################################################################

import math
import os.path
import sys


try:
    from PIL import Image, ImageDraw, ImageFilter, ImageEnhance, ImageChops

except ImportError as error:
    print(
        f"Unable to import from PIL, please install Pillow 7.0.0+ (python3-pillow) - error: {error}",
        file=sys.stderr
    )
    sys.exit(1)


def main(argv):
    knob_bg = Image.open(os.path.join(os.path.dirname(argv[0]), "../gui/knob.png"))

    knob_ticks_color = (144, 144, 150)
    knob_line_color = (200, 200, 220)

    generate_knob_states(
        knob_bg=knob_bg,
        glow_color_1=(72, 0, 55),
        glow_color_2=(0, 42, 144),
        glow_color_3=(80, 170, 255),
        ticks_color=knob_ticks_color,
        line_color=knob_line_color,
        out_file=os.path.join(os.path.dirname(argv[0]), "../gui/img/knob_states-free.png")
    )


def generate_knob_states(
        knob_bg,
        glow_color_1,
        glow_color_2,
        glow_color_3,
        ticks_color,
        line_color,
        out_file,
        stages=128
):

    width, height = knob_bg.size
    center = (int(round(width / 2)), int(round(height / 2)))
    angle_diff = 30.0

    # in PIL, 3 o'clock is 0 degrees
    start_angle = 90.0 + angle_diff
    end_angle_delta = 360.0 - angle_diff * 2.0
    arc_distance = 28

    overlay = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    canvas = ImageDraw.Draw(overlay)

    for i in range(15):
        progress = float(i) / 14.0
        p1 = rotate((center[0], 1.0), center, end_angle_delta * (progress - 0.5))
        p2 = rotate((center[0], 12.0), center, end_angle_delta * (progress - 0.5))
        canvas.line((p1, p2), ticks_color, 6)

    knob_bg.alpha_composite(overlay, (0, 0), (0, 0, width, height))

    knob_states = Image.new("RGB", (width, height * stages), (0, 0, 0))
    knob_states_canvas = ImageDraw.Draw(knob_states)
    glow_color = glow_color_1

    for i in range(stages):
        progress = float(i) / float(stages - 1) if stages > 1 else 0.0
        angle_delta = end_angle_delta * progress
        p1 = rotate(
            (center[0], 24), center, end_angle_delta * (progress - 0.5)
        )
        p2 = rotate(
            (center[0], 103), center, end_angle_delta * (progress - 0.5)
        )

        if progress < 0.5:
            glow_color = vsum(
                vscale(1.0 - 2.0 * progress, glow_color_1),
                vscale(2.0 * progress, glow_color_2)
            )
        else:
            glow_color = vsum(
                vscale(1.0 - 2.0 * (progress - 0.5), glow_color_2),
                vscale(2.0 * (progress - 0.5), glow_color_3)
            )

        overlay = Image.new("RGBA", (width, height), (0, 0, 0, 0))
        canvas = ImageDraw.Draw(overlay)
        canvas.arc(
            (arc_distance, arc_distance, width - arc_distance, height - arc_distance),
            start_angle,
            start_angle + angle_delta,
            (int(glow_color[0]), int(glow_color[1]), int(glow_color[2])),
            25
        )
        overlay_glow = overlay.copy()

        canvas.arc(
            (arc_distance, arc_distance, width - arc_distance, height - arc_distance),
            start_angle,
            start_angle + end_angle_delta,
            (0, 5, 9, 190),
            25
        )

        if line_color is not None:
            canvas.line((p2, p1), line_color, 15)

        knob = Image.new("RGBA", (width, height), (0, 0, 0, 0))
        knob.paste(knob_bg, (0, 0, width, height))
        overlay = overlay.filter(ImageFilter.GaussianBlur(2))
        knob.alpha_composite(overlay, (0, 0), (0, 0, width, height))

        overlay_glow = ImageEnhance.Brightness(overlay_glow).enhance(1.5)
        overlay_glow = ImageEnhance.Contrast(overlay_glow).enhance(1.7)
        overlay_glow = overlay_glow.filter(ImageFilter.GaussianBlur(12))
        knob = ImageChops.blend(
            knob,
            ImageChops.screen(knob, overlay_glow),
            1.0
        )

        knob_states.paste(knob.copy(), (0, height * i, width, height * (i + 1)))

    knob_states = knob_states.resize(
        (int(width / 5), int(height / 5) * stages), Image.BICUBIC
    )
    knob_states.save(out_file)

    return 0


def rotate(point, origin, degrees):
    rad = (degrees * math.pi) / 180.0
    sin = math.sin(rad)
    cos = math.cos(rad)
    x, y = vdiff(point, origin)

    return vsum(origin, (x * cos - y * sin, x * sin + y * cos))


def vsum(a, b):
    return tuple(ai + bi for ai, bi in zip(a, b))


def vdiff(a, b):
    return tuple(ai - bi for ai, bi in zip(a, b))


def vscale(scalar, vector):
    return tuple(scalar * vi for vi in vector)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
