#include <Python.h>
#include <QApplication>
#include <QSurfaceFormat>

#include "MainWindow.h"

#include "core/define.h"
//#include "core/unittest.h"
#include "core/kernel.h"
#include "core/module.h"
#include "Modeler.h"
#include "TemplateNode.h"
#include "PostNode.h"
#include "PostPlotNode.h"
#include "bzmagpy/pythonscriptserver.h"

INCLUDE_MODULE(ENGINELIBRARY_API, Engine);
INCLUDE_MODULE(PYLIBLIBRARY_API, bzmagPy);

extern PyObject* g_module;

int main(int argc, char** argv) 
{
    //_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
    //_CrtSetBreakAlloc(320);

    USING_MODULE(Engine);
    USING_MODULE(bzmagPy);

    bzmag::Module module("bzMagEditor", 0, 0);
    bzmag::Module* m = &module;
    REGISTER_TYPE(m, Modeler);
    REGISTER_TYPE(m, TemplateNode);
    REGISTER_TYPE(m, PostNode);
    REGISTER_TYPE(m, PostPlotNode);
    Modeler::setSingletonPath("/sys/modeler");

    // 모듈을 sys.modules에 등록
    PyObject* sys_modules = PyImport_GetModuleDict();
    PyDict_SetItemString(sys_modules, "bzmagPy", g_module);

    // HiDPI 환경 설정
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  // Qt 5.6 이상에서 사용 가능
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);      // 픽셀맵도 HiDPI 적용
    QApplication app(argc, argv);

    QFont font; 
    font.setFamily(QString("맑은 고딕"));
    app.setFont(font);  // 애플리케이션 기본 글꼴 설정

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(4, 6);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    app.setWindowIcon(QIcon(":/bzmagEdior/icons/bzmag_icon.ico"));

    MainWindow window;
    window.show();
    window.connectModeler();
    return app.exec();
}