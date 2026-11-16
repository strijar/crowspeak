#!/usr/bin/env python3
"""Run the actual LVGL styles and touch keyboard on the host, without hardware."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import os
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
LVGL = ROOT / ".pio/libdeps/crowpanel/lvgl"
UI = ROOT / "src/ui/lvgl"


def run(command):
    subprocess.run(command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def main():
    if not LVGL.is_dir():
        raise SystemExit("Install project dependencies first: make setup")
    with tempfile.TemporaryDirectory(prefix="crowspeak-ui-") as temporary:
        build = Path(temporary)
        config = (UI / "lv_conf.h").read_text()
        config = re.sub(r"^#define LV_(MEM_CUSTOM\w*|TICK_CUSTOM\w*|ASSERT_HANDLER\w*).*\n", "", config, flags=re.M)
        config = config.replace("#endif // LV_CONF_H", """
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (2 * 1024 * 1024)
#define LV_TICK_CUSTOM 0
#define LV_USE_ASSERT_STYLE 1
#endif
""")
        (build / "lv_conf.h").write_text(config)
        # Exercise production Peers allocation/entry/teardown on native LVGL.
        # Platform services are substituted by peers_lifecycle_smoke.cpp.
        declaration = (UI / "screens/LvNodesScreen.h").read_text()
        declaration = re.sub(r"^#include .*\n", "", declaration, flags=re.M)
        (build / "peer_screen_declaration.h").write_text(declaration)
        screen = (UI / "screens/LvNodesScreen.cpp").read_text()
        methods = []
        for name in ("createUI", "onEnter", "onHide", "destroyUI"):
            start = screen.index("void LvNodesScreen::" + name + "(")
            end = screen.index("\n}", start) + 2
            methods.append(screen[start:end])
        (build / "peer_screen_lifecycle.inc").write_text("\n\n".join(methods))
        flags = ["-O0", "-g", "-DLV_CONF_INCLUDE_SIMPLE", "-I" + str(build),
                 "-I" + str(LVGL), "-I" + str(UI), "-I" + str(ROOT / "src/core"), "-I" + str(ROOT / "src/boards/crowpanel")]
        sources = list((LVGL / "src").rglob("*.c"))
        sources += list((UI / "fonts").glob("lv_font_crowpanel_*.c"))
        sources += [UI / "Style.cpp", UI / "Theme.cpp", UI / "TouchKeyboard.cpp", UI / "LvTabBar.cpp", ROOT / "tools/tests/ui_styles_smoke.cpp", ROOT / "tools/tests/peers_lifecycle_smoke.cpp"]

        def compile_source(item):
            index, source = item
            obj = build / (str(index) + ".o")
            cpp = source.suffix == ".cpp"
            run(["c++" if cpp else "cc", "-std=gnu++17" if cpp else "-std=gnu11",
                 *flags, "-c", str(source), "-o", str(obj)])
            return str(obj)

        try:
            with ThreadPoolExecutor(max_workers=min(os.cpu_count() or 2, 8)) as pool:
                objects = list(pool.map(compile_source, enumerate(sources)))
            run(["c++", *objects, "-lm", "-o", str(build / "smoke")])
            subprocess.run([str(build / "smoke")], check=True)
        except subprocess.CalledProcessError as error:
            if error.stderr:
                print(error.stderr.decode(errors="replace"))
            raise


if __name__ == "__main__":
    main()
