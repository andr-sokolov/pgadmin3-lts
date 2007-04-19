//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id$
// Copyright (C) 2002 - 2007, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// wsMainFrame.cpp - debugger
//
//////////////////////////////////////////////////////////////////////////

#include "wsMainFrame.h"
#include "wsCodeWindow.h"
#include "wsPgconn.h"
#include "wsDirectdbg.h"
#include "debugger.h"
#include "debuggerMenu.h"

#include "images/debugger.xpm"
#include "images/clearAll.xpm"
#include "images/continue.xpm"
#include "images/setBreak.xpm"
#include "images/stepOver.xpm"
#include "images/stepInto.xpm"
#include "images/stop.xpm"

#include <wx/log.h>

// Copyright text(pgadmin)
#include "../../pgadmin/include/copyright.h"
#include "../../pgadmin/include/version.h"
#include "../../pgadmin/include/svnversion.h"

#define VERSION_WITH_DATE_AND_SVN       wxT("Version ") VERSION_STR wxT(" (") __TDATE__ wxT(", rev: ") wxT(VERSION_SVN) wxT(")")

IMPLEMENT_CLASS( wsMainFrame, wxWindow  )

BEGIN_EVENT_TABLE( wsMainFrame, wxWindow  )
    EVT_MENU(wxID_ABOUT, wsMainFrame ::OnAbout)
    EVT_MENU(wxID_EXIT,  wsMainFrame ::OnExit)

    EVT_MENU_RANGE(MENU_ID_TOGGLE_BREAK, MENU_ID_STOP, wsMainFrame::OnDebugCommand)
    EVT_CLOSE(wsMainFrame::OnClose)
    EVT_SIZE(wsMainFrame::OnSize)

    EVT_STC_MARGINCLICK(wxID_ANY,        wsMainFrame::OnMarginClick)
    EVT_LISTBOX(wxID_ANY,                wsMainFrame::OnSelectFrame)

    EVT_MENU(MENU_ID_VIEW_TOOLBAR,       wsMainFrame::OnToggleToolBar)
    EVT_MENU(MENU_ID_VIEW_STACKPANE,     wsMainFrame::OnToggleStackPane)
    EVT_MENU(MENU_ID_VIEW_OUTPUTPANE,    wsMainFrame::OnToggleOutputPane)
    EVT_MENU(MENU_ID_VIEW_DEFAULTVIEW,   wsMainFrame::OnDefaultView)
    EVT_AUI_PANE_CLOSE(wsMainFrame::OnAuiUpdate)
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////
// wsMainFrame constructor
//
//  wsMainFrame manages the user interface for the workstation. This class
//  manages the toolbar, menu, status bar, and top-level windows.  

wsMainFrame::wsMainFrame( wxWindow *parent, const wxString &title, const wxPoint & pos, const wxSize & size )
	: wxFrame( parent, wxID_ANY, title, pos, size ),
	  m_standaloneDebugger( NULL )
{
    wxWindowBase::SetFont(glApp->GetSystemFont());

	manager.SetManagedWindow(this);
	manager.SetFlags(wxAUI_MGR_DEFAULT | wxAUI_MGR_TRANSPARENT_DRAG);

	// Define the icon for this window
	SetIcon(wxIcon(debugger_xpm));

	// Create (and configure) the menu bar, toolbar, and status bar
	m_menuBar   = setupMenuBar();
	m_toolBar   = setupToolBar();
	m_statusBar = setupStatusBar();

	manager.AddPane(m_toolBar, wxAuiPaneInfo().Name(wxT("toolBar")).Caption(wxT("Toolbar")).ToolbarPane().Top().Row(1).Position(1).LeftDockable(false).RightDockable(false));

    // Now load the layout
    wxString perspective;
    glApp->getSettings().Read(wxT("Debugger/wsMainFrame/Perspective-") + VerFromRev(WSMAINFRAME_PERPSECTIVE_VER), &perspective, WSMAINFRAME_DEFAULT_PERSPECTIVE);
    manager.LoadPerspective(perspective, true);

    // and reset the captions for the current language
    manager.GetPane(wxT("toolBar")).Caption(_("Toolbar"));

    // Sync the View menu options
    m_viewMenu->Check(MENU_ID_VIEW_TOOLBAR, manager.GetPane(wxT("toolBar")).IsShown());

	manager.Update();

}

wsMainFrame::~wsMainFrame()
{
    glApp->getSettings().Write(wxT("Debugger/wsMainFrame/Perspective-") + VerFromRev(WSMAINFRAME_PERPSECTIVE_VER), manager.SavePerspective());
}

////////////////////////////////////////////////////////////////////////////////
// VerFromRev()
//
// 	Get a version number from a revision string
wxString wsMainFrame::VerFromRev(const wxString &rev)
{
   wxString ret = rev.AfterFirst(' ');
   ret = ret.BeforeFirst(' ');
   return ret;
}

////////////////////////////////////////////////////////////////////////////////
// addDebug()
//
// 	This function creates a new debugger window...

wsCodeWindow * wsMainFrame::addDebug( const wsConnProp & connProps )
{
    m_standaloneDebugger = new wsCodeWindow( this , -1, connProps );
	m_standaloneDebugger->Show( false );
    return( m_standaloneDebugger );
}

wsDirectDbg * wsMainFrame::addDirectDbg( const wsConnProp & connProp )
{

	m_standaloneDirectDbg = new wsDirectDbg( this, -1, connProp );
	m_standaloneDirectDbg->Show( true );
	return( m_standaloneDirectDbg );
}

////////////////////////////////////////////////////////////////////////////////
// OnDebugCommand()
//
// 	This event handler is invoked when the user clicks one of the debugger 
// 	tools (on the debugger toolbar) - we simply forward the event to the 
//  debugger window.

void wsMainFrame::OnDebugCommand( wxCommandEvent & event )
{
    m_standaloneDebugger->OnCommand( event );
}

////////////////////////////////////////////////////////////////////////////////
// OnSelectFrame()
//
// 	This event handler is invoked when the user clicks one of the stack frames
//  - we simply forward the event to the debugger window.

void wsMainFrame::OnSelectFrame( wxCommandEvent & event )
{
    m_standaloneDebugger->OnSelectFrame( event );
}

////////////////////////////////////////////////////////////////////////////////
// OnMarginClick()
//
// 	This event handler is invoked when the user clicks one of the stack frames
//  - we simply forward the event to the debugger window.

void wsMainFrame::OnMarginClick( wxStyledTextEvent & event )
{
    m_standaloneDebugger->OnMarginClick( event );
}

void wsMainFrame ::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxString Versions( VERSION_WITH_DATE_AND_SVN );
	Versions += wxT("\n");
	Versions += COPYRIGHT;
	Versions += wxT("\n");
	Versions += LICENSE;
	wxMessageBox(Versions, _("Debugger"), wxOK, this);
}

////////////////////////////////////////////////////////////////////////////////
// OnSize()
//
//	This event handler is called when a resize event occurs (that is, when 
//  wxWidgets needs to size the application window)

void wsMainFrame::OnSize( wxSizeEvent & event )
{
	event.Skip();
}

////////////////////////////////////////////////////////////////////////////////
// setupToolBar()
//
//	This function creates the standard toolbar

wxToolBar * wsMainFrame::setupToolBar( void )
{
	wxToolBar *t = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);

	t->SetToolBitmapSize( wxSize( 16, 16 ));
 
	t->AddTool( MENU_ID_STEP_INTO,		 _( "Step into" ),    		 wxBitmap(stepInto_xpm));
	t->AddTool( MENU_ID_STEP_OVER,		 _( "Step over" ),    		 wxBitmap(stepOver_xpm));
	t->AddTool( MENU_ID_CONTINUE,		 _( "Continue" ),    		 wxBitmap(continue_xpm));
    t->AddSeparator();
	t->AddTool( MENU_ID_TOGGLE_BREAK,	 _( "Toggle breakpoint" ),   wxBitmap(setBreak_xpm));
	t->AddTool( MENU_ID_CLEAR_ALL_BREAK, _( "Clear all breakpoints" ), wxBitmap(clearAll_xpm));
    t->AddSeparator();
	t->AddTool( MENU_ID_STOP,      		 _( "Stop debugging" ),    	 wxBitmap(stop_xpm));

	t->Realize();
	m_toolBar = t;

	return( m_toolBar );
}

////////////////////////////////////////////////////////////////////////////////
// setupStatusBar()
//
//	This function initializes the status bar (we don't have one at the moment
//  so this function simply returns)

wxStatusBar * wsMainFrame::setupStatusBar( void )
{
	wxStatusBar * bar = CreateStatusBar( 3, wxST_SIZEGRIP );
	int			  widths[] = { 0, -1, -1 };

	bar->SetStatusWidths(3, widths);
	bar->SetStatusText(_( "Initializing..."), 1);

    return( bar );
}

////////////////////////////////////////////////////////////////////////////////
// setupMenuBar()
//
//	This function creates the standard menu bar

wxMenuBar *wsMainFrame::setupMenuBar(void)
{
	m_menuBar = new wxMenuBar;

	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_EXIT, _("E&xit"));
	m_menuBar->Append(fileMenu, _("&File"));

	m_debugMenu = new wxMenu;
	m_debugMenu->Append(MENU_ID_STEP_INTO, _( "Step into\tF11" ));
	m_debugMenu->Append(MENU_ID_STEP_OVER, _( "Step over\tF10" ));
    m_debugMenu->Append(MENU_ID_CONTINUE, _( "Continue\tF5" ));
    m_debugMenu->AppendSeparator();
    m_debugMenu->Append(MENU_ID_TOGGLE_BREAK, _( "Toggle breakpoint\tF9" ));
    m_debugMenu->Append(MENU_ID_CLEAR_ALL_BREAK, _( "Clear all breakpoints\tCtrl+Shift+F9" ));
    m_debugMenu->AppendSeparator();
    m_debugMenu->Append(MENU_ID_STOP, _( "Stop debugging\tF8" ));
	m_menuBar->Append(m_debugMenu, _("&Debug"));

	m_viewMenu = new wxMenu;
    m_viewMenu->Append(MENU_ID_VIEW_OUTPUTPANE, _("&Output pane"), _("Show or hide the output pane."), wxITEM_CHECK);
    m_viewMenu->Append(MENU_ID_VIEW_STACKPANE, _("&Stack pane"),   _("Show or hide the stack pane."), wxITEM_CHECK);
    m_viewMenu->Append(MENU_ID_VIEW_TOOLBAR, _("&Tool bar"),       _("Show or hide the tool bar."), wxITEM_CHECK);
    m_viewMenu->AppendSeparator();
    m_viewMenu->Append(MENU_ID_VIEW_DEFAULTVIEW, _("&Default view"),     _("Restore the default view."));
	m_menuBar->Append(m_viewMenu, _("&View"));

	wxMenu* help_menu = new wxMenu;
	help_menu->Append(wxID_ABOUT, _("&About..."));
	m_menuBar->Append(help_menu, _("&Help"));


	SetMenuBar( m_menuBar );

	return( m_menuBar );
}

////////////////////////////////////////////////////////////////////////////////
// OnClose()
//
//	wxWidgets invokes this event handler when the user closes the main window

void wsMainFrame::OnClose( wxCloseEvent & event )
{
	manager.UnInit();
	Destroy();
}

////////////////////////////////////////////////////////////////////////////////
// OnExit()
//
//	Close the debugger

void wsMainFrame::OnExit( wxCommandEvent & event )
{
	this->Close();
}


////////////////////////////////////////////////////////////////////////////////
//  OnToggleToolBar()
//
//	Turn the tool bar on or off
void wsMainFrame::OnToggleToolBar(wxCommandEvent& event)
{
    if (m_viewMenu->IsChecked(MENU_ID_VIEW_TOOLBAR))
        manager.GetPane(wxT("toolBar")).Show(true);
    else
        manager.GetPane(wxT("toolBar")).Show(false);
    manager.Update();
}

////////////////////////////////////////////////////////////////////////////////
//  OnToggleStackPane()
//
//	Turn the tool bar on or off
void wsMainFrame::OnToggleStackPane(wxCommandEvent& event)
{
    if (m_viewMenu->IsChecked(MENU_ID_VIEW_STACKPANE))
        manager.GetPane(wxT("stackPane")).Show(true);
    else
        manager.GetPane(wxT("stackPane")).Show(false);
    manager.Update();
}

////////////////////////////////////////////////////////////////////////////////
//  OnToggleOutputPane()
//
//	Turn the tool bar on or off
void wsMainFrame::OnToggleOutputPane(wxCommandEvent& event)
{
    if (m_viewMenu->IsChecked(MENU_ID_VIEW_OUTPUTPANE))
        manager.GetPane(wxT("outputPane")).Show(true);
    else
        manager.GetPane(wxT("outputPane")).Show(false);
    manager.Update();
}

////////////////////////////////////////////////////////////////////////////////
//  OnAuiUpdate()
//
//	Update the view menu to reflect AUI changes
void wsMainFrame::OnAuiUpdate(wxAuiManagerEvent& event)
{
    if(event.pane->name == wxT("toolBar"))
    {
        m_viewMenu->Check(MENU_ID_VIEW_TOOLBAR, false);
    }
    else if(event.pane->name == wxT("stackPane"))
    {
        m_viewMenu->Check(MENU_ID_VIEW_STACKPANE, false);
    }
    else if(event.pane->name == wxT("outputPane"))
    {
        m_viewMenu->Check(MENU_ID_VIEW_OUTPUTPANE, false);
    }
    event.Skip();
}

////////////////////////////////////////////////////////////////////////////////
//  OnDefaultView()
//
//	Reset the AUI view to the default
void wsMainFrame::OnDefaultView(wxCommandEvent& event)
{
    manager.LoadPerspective(WSMAINFRAME_DEFAULT_PERSPECTIVE, true);

    // Reset the captions for the current language
    manager.GetPane(wxT("toolBar")).Caption(_("Toolbar"));
    manager.GetPane(wxT("stackPane")).Caption(_("Stack pane"));
    manager.GetPane(wxT("outputPane")).Caption(_("Output pane"));

    // tell the manager to "commit" all the changes just made
    manager.Update();

    // Sync the View menu options
    m_viewMenu->Check(MENU_ID_VIEW_TOOLBAR, manager.GetPane(wxT("toolBar")).IsShown());
    m_viewMenu->Check(MENU_ID_VIEW_STACKPANE, manager.GetPane(wxT("stackPane")).IsShown());
    m_viewMenu->Check(MENU_ID_VIEW_OUTPUTPANE, manager.GetPane(wxT("outputPane")).IsShown());
}
