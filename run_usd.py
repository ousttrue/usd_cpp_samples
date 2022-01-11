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

# setup USD
sys.path.insert(0, str(HERE / 'build_release/lib/python'))
os.environ['PATH'] = f'{HERE / "build_release/bin"};{HERE / "build_release/lib"};{os.environ["PATH"]}'
from pxr import Usd  # nopep8
from pxr import UsdImagingGL  # nopep8
from pxr.Gf import Vec4f, Vec4d, Camera, Matrix4d, Vec3d  # nopep8
from pxr.Glf import SimpleMaterial  # nopep8


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
    def __init__(self, scale: float = 1):
        super().__init__(scale=scale)

        # load usd
        # path = pathlib.Path(os.environ['USERPROFILE']) / \
        #     "Desktop/Kitchen_set/Kitchen_set.usd"
        path = HERE / 'test_sphere.usda'
        assert path.exists()

        self.engine = None

        self.stage: Usd.Stage = Usd.Stage.Open(str(path))

        self.params = UsdImagingGL.RenderParams()
        self.params.clearColor = Vec4f(1.0, 0.5, 0.1, 1.0)
        # params.frame = Usd.TimeCode.Default()
        self.params.frame = 1.0
        self.params.complexity = 1.0
        self.params.forceRefresh = False
        self.params.enableLighting = False
        # params.enableLighting = True
        self.params.enableSceneMaterials = True
        self.params.drawMode = UsdImagingGL.DrawMode.DRAW_SHADED_SMOOTH
        self.params.highlight = True
        self.params.gammaCorrectColors = False
        # params.colorCorrectionMode = TfToken("sRGB");
        self.params.showGuides = True
        self.params.showProxy = True
        self.params.showRender = False
        self.params.forceRefresh = True

        material = SimpleMaterial()
        ambient = Vec4f()

        self.camera = Camera()
        # self.camera.SetPerspectiveFromAspectRatioAndFieldOfView(16.0 / 9.0, 60, Camera.FOVHorizontal)
        # self.camera.focus_distance = 100
        dist = 100
        trans = Matrix4d()
        trans.SetTranslate(Vec3d.ZAxis() * dist)
        # GfMatrix4d roty(1.0);
        # GfMatrix4d rotz(1.0);
        # GfMatrix4d rotx(1.0);
        # roty.SetRotate(GfRotation(GfVec3d::YAxis(), -rotation[0]));
        # rotx.SetRotate(GfRotation(GfVec3d::XAxis(), -rotation[1]));
        # rotz.SetRotate(GfRotation(GfVec3d::ZAxis(), -rotation[2]));
        # GfMatrix4d toCenter;
        # toCenter.SetTranslate(center);
        self.camera.transform = trans
        self.camera.focus_distance = dist

    def imgui_create_docks(self):
        return ImguiDocks()

    def lazy_init(self):
        if self.engine:
            return
        self.engine = UsdImagingGL.Engine()

    def draw_scene(self):
        self.lazy_init()
        frustum = self.camera.frustum
        # engine.SetLightingState(_lights, _material, _ambient);
        self.engine.SetRenderViewport(Vec4d(0, 0, *controller.viewport))
        # engine.SetWindowPolicy(CameraUtilConformWindowPolicy::CameraUtilMatchHorizontally);
        # If using a usd camera, use SetCameraPath renderer.SetCameraPath(sceneCam.GetPath())
        # else set camera state
        self.engine.SetCameraState(frustum.ComputeViewMatrix(
        ), frustum.ComputeProjectionMatrix())
        self.engine.Render(self.stage.GetPseudoRoot(), self.params)


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

    # main loop
    lastCount = 0
    while True:
        count = loop.begin_frame()
        if not count:
            break

        d = count - lastCount
        lastCount = count
        if d > 0:
            controller.onUpdate(d)
            controller.draw()

            loop.end_frame()

    # save window config
    config = loop.get_config()
    config.save_json_to_path(CONFIG_FILE)
