#include <QtGui/QApplication>

#include "main.h"
#include "mainwindow.h"
#include "cpluginmanager.h"
#include "startupsplashdialog.h"
#include "environmentsettingsdialog.h"

// Thread for NES emulator.  This thread runs the NES core.
NESEmulatorThread* emulator = NULL;

// Modeless dialog for Test Suite executive.
TestSuiteExecutiveDialog* testSuiteExecutive = NULL;

// Interface to compiler.
CompilerThread* compiler = NULL;

// Interface to search engine.
SearcherThread* searcher = NULL;

// Thread for watching for breakpoints ejected by the NES
// emulator thread.
BreakpointWatcherThread* breakpointWatcher = NULL;

// Hash table of debugger inspector windows to support automagic
// opening/closing of inspector windows on things like breakpoints.
QHash<QString,QDockWidget*> inspectors;

// Database of all known games.
CGameDatabaseHandler gameDatabase;

// Main window of application.
MainWindow* nesicideWindow;

// The project container.
CNesicideProject* nesicideProject = (CNesicideProject*)NULL;

int main(int argc, char* argv[])
{
   QApplication nesicideApplication(argc, argv);

   QCoreApplication::setOrganizationName("CSPSoftware");
   QCoreApplication::setOrganizationDomain("nesicide.com");
   QCoreApplication::setApplicationName("NESICIDE");

   // Initialize Environment settings.
   EnvironmentSettingsDialog::readSettings();

   // Initialize the game database object...
   if ( EnvironmentSettingsDialog::useInternalGameDatabase() )
   {
      // Use internal resource.
      gameDatabase.initialize(":GameDatabase");
   }
   else
   {
      // Use named file resource.  Default to internal if it's not set.
      gameDatabase.initialize(EnvironmentSettingsDialog::getGameDatabase());
   }

   // Initialize the plugin manager
   pluginManager = new CPluginManager();

   // Run the startup splash
   StartupSplashDialog* splash = new StartupSplashDialog();
   //splash->exec();
   delete splash;

   // Set up default OpenGL format.
   QGLFormat fmt = QGLFormat::defaultFormat();

   // Disable VSYNC waiting.
   fmt.setSwapInterval(0);

   QGLFormat::setDefaultFormat(fmt);

   // Create, show, and execute the main window (UI) thread.
   nesicideWindow = new MainWindow();
   nesicideWindow->show();

   int result = nesicideApplication.exec();

   delete pluginManager;
   pluginManager = NULL;

   return result;
}
