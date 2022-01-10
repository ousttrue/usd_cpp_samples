import pathlib
import os
import sys
import types
import importlib
from typing import Iterable, Dict, Optional, List
import logging
import pathlib
import ctypes
#
import glglue.glfw
from glglue.gl3.pydearcontroller import PydearController
from glglue.gl3.renderview import RenderView
from glglue.windowconfig import WindowConfig
import pydear as ImGui
from pydear.utils.dockspace import DockView
from glglue.scene.node import Node
from glglue.ctypesmath import TRS, Mat4, Camera

logger = logging.getLogger(__name__)

CONFIG_FILE = pathlib.Path("window.ini")

HERE = pathlib.Path(__file__).absolute().parent

sys.path.insert(0, str(HERE / 'build_release/lib/python'))
os.environ['PATH'] = f'{HERE / "build_release/bin"};{HERE / "build_release/lib"};{os.environ["PATH"]}'


class ImguiDocks:
    def __init__(self) -> None:
        self.demo = DockView(
            'demo', (ctypes.c_bool * 1)(True), ImGui.ShowDemoWindow)
        self.metrics = DockView(
            'metrics', (ctypes.c_bool * 1)(True), ImGui.ShowMetricsWindow)
        self.selected = 'skin'

        # logger
        from pydear.utils.loghandler import ImGuiLogHandler
        log_handle = ImGuiLogHandler()
        log_handle.register_root()
        self.logger = DockView('log', (ctypes.c_bool * 1)
                               (True), log_handle.draw)

    def __iter__(self) -> Iterable[DockView]:
        yield self.demo
        yield self.metrics
        yield self.logger


class SampleController(PydearController):
    def imgui_create_docks(self):
        return ImguiDocks()


if __name__ == "__main__":
    logging.basicConfig(
        format='%(levelname)s:%(name)s:%(message)s', level=logging.DEBUG)

    # ImGui
    controller = SampleController()

    # glfw
    loop = glglue.glfw.LoopManager(
        controller,
        config=WindowConfig.load_json_from_path(CONFIG_FILE),
        title="pydear")

    path = pathlib.Path(os.environ['USERPROFILE']) / \
        "Desktop/Kitchen_set/Kitchen_set.usd"
    assert path.exists()

    from pxr import Usd
    stage: Usd.Stage = Usd.Stage.Open(str(path))

    from pxr import UsdImagingGL
    engine = UsdImagingGL.Engine()
    renderParams = UsdImagingGL.RenderParams()

    # main loop
    lastCount = 0
    while True:
        count = loop.begin_frame()
        if not count:
            break

        # _renderer->SetCameraState(GetCurrentCamera().GetFrustum().ComputeViewMatrix(),GetCurrentCamera().GetFrustum().ComputeProjectionMatrix());
        engine.Render(stage.GetPseudoRoot(), renderParams)

        d = count - lastCount
        lastCount = count
        if d > 0:
            controller.onUpdate(d)
            controller.draw()
            loop.end_frame()

    # save window config
    config = loop.get_config()
    config.save_json_to_path(CONFIG_FILE)
