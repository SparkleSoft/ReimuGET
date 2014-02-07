
// ReimuGETDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReimuGET.h"
#include "ReimuGETDlg.h"
#include "afxdialogex.h"

#include <iostream>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif 

#define BUFSIZE 4096 
 
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hInputFile = NULL;

// Prototypes
UINT DownloadFiles(LPVOID pParam);
UINT CheckLog(LPVOID pParam);

bool isDling = false, queueRunning = false, assumeError = true, killAria = false, ExitOK = false;
int nItem = 0;

// CAboutDlg dialog used for App About

int CALLBACK SortItemURLs(LPARAM lParam1, LPARAM lParam2, 
	LPARAM lParamSort)
{
	UNREFERENCED_PARAMETER(lParamSort);

	return (int)(lParam1 - lParam2);
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CReimuGETDlg dialog




CReimuGETDlg::CReimuGETDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CReimuGETDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CReimuGETDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST3, m_FileQueue);
	DDX_Control(pDX, IDC_EDIT1, m_URL);
	DDX_Control(pDX, IDC_SLIDER1, m_Connections);
	DDX_Control(pDX, IDC_PROGRESS1, m_progBar);
}

BEGIN_MESSAGE_MAP(CReimuGETDlg, CDialogEx)

	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON2, &CReimuGETDlg::OnBnClickedButton2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CReimuGETDlg::OnNMCustomdrawSlider1)
	ON_BN_CLICKED(IDC_BUTTON3, &CReimuGETDlg::OnBnClickedButton3)
	ON_NOTIFY(LVN_INSERTITEM, IDC_LIST3, &CReimuGETDlg::OnLvnInsertitemList3)
//	ON_WM_CLOSE()
ON_WM_CLOSE()
END_MESSAGE_MAP()


// CReimuGETDlg message handlers

BOOL CReimuGETDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon






	HGLOBAL hResourceLoaded;  // handle to loaded resource
    HRSRC   hRes;              // handle/ptr to res. info.
    char    *lpResLock;        // pointer to resource data
    DWORD   dwSizeRes;
    hRes = FindResource(NULL,MAKEINTRESOURCE(IDR_BIN1),CString(L"BIN"));
    hResourceLoaded = LoadResource(NULL, hRes);
    lpResLock = (char *) LockResource(hResourceLoaded);
    dwSizeRes = SizeofResource(NULL, hRes);
	FILE * outputRes;
	outputRes = _wfopen(L"ReimuGET_temp_aria2c.exe", L"wb");
	fwrite ((const char *) lpResLock,1,dwSizeRes,outputRes);
	fclose(outputRes);



	CWinThread* pCheckLog = AfxBeginThread(CheckLog,THREAD_PRIORITY_NORMAL);

	m_Connections.SetRange(1, 16);
	m_Connections.SetPos(16);

	LVCOLUMN lvColumn;
	
	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 80;
	lvColumn.pszText = L"Status";
	m_FileQueue.InsertColumn(1, &lvColumn);

	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 80;
	lvColumn.pszText = L"Connections";
	m_FileQueue.InsertColumn(1, &lvColumn);

	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 294;
	lvColumn.pszText = L"URL";
	m_FileQueue.InsertColumn(2, &lvColumn);

	m_progBar.SetRange(0, 100);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CReimuGETDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CReimuGETDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CReimuGETDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CReimuGETDlg::OnBnClickedButton2()
{
	// Add URL:

	CString dlgURL;
	GetDlgItemText(IDC_EDIT1, dlgURL);
	
	if (dlgURL != L"") {
		SetDlgItemText(IDC_EDIT1, L"");

		LVITEM lvItem;

		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = ++nItem;
		nItem++;
		lvItem.iSubItem = 0;
		lvItem.pszText = L"Queued";
		nItem = m_FileQueue.InsertItem(&lvItem);

		CString connPos;
		connPos.Format(L"%i", m_Connections.GetPos());

		m_FileQueue.SetItemText(nItem, 1, connPos);
		m_FileQueue.SetItemText(nItem, 2, dlgURL);
	} else {
		AfxMessageBox(L"Stop failing!");
	}
}


void CReimuGETDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);


	CString connPos;
	connPos.Format(L"%i", m_Connections.GetPos());

	SetDlgItemText(IDC_CONN, connPos);
	
	// TODO: Add your control notification handler code here
	
	
	*pResult = 0;
}

UINT DownloadFiles(LPVOID pParam) {	
	CWnd* pwnd = AfxGetMainWnd(); // Pointer to main window

	// Pointer to our progress bar....must have..
	CListCtrl* m_FileQueue = (CListCtrl*)pwnd->GetDlgItem(IDC_LIST3);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa; 

	sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
	sa.bInheritHandle = TRUE; 
	sa.lpSecurityDescriptor = NULL; 

	if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0) ) 
	exit(1); 

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
	exit(1); 

	// Create a pipe for the child process's STDIN. 

	if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &sa, 0)) 
	exit(1); 

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
	exit(1); 

	CButton* bGetFile;

	bGetFile = (CButton*)pwnd->GetDlgItem(IDC_BUTTON3);
	bGetFile->EnableWindow(FALSE);

	CProgressCtrl* m_Prog = (CProgressCtrl*)pwnd->GetDlgItem(IDC_PROGRESS1);

	queueRunning = true;

	for (int i = 0; i < m_FileQueue->GetItemCount(); i++){
		if (m_FileQueue->GetItemText(i, 0) != L"Done") {
			assumeError = true;

			DWORD exitCode = 0;
			ZeroMemory( &si, sizeof(si) );
			si.cb = sizeof(si);
			ZeroMemory( &pi, sizeof(pi) );

			si.hStdError = g_hChildStd_OUT_Wr;
			si.hStdOutput = g_hChildStd_OUT_Wr;
			si.hStdInput = g_hChildStd_IN_Rd;

			si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; // STARTF_USESTDHANDLES is Required.
			si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

			CString DownloadDir;
			pwnd->GetDlgItemText(IDC_MFCEDITBROWSE1, DownloadDir);

			
			if (DownloadDir == L"") {
				DownloadDir = L".";
			}

			int Value = CreateProcess(NULL, CString(L"ReimuGET_temp_aria2c.exe --dir " + DownloadDir + L" --max-connection-per-server " + m_FileQueue->GetItemText(i, 1) + L" --min-split-size 1M --split " + m_FileQueue->GetItemText(i, 1) + L" " + m_FileQueue->GetItemText(i, 2)).GetBuffer(), NULL, NULL, true, 0, NULL, NULL, &si, &pi);
						
			
			m_FileQueue->SetItemText(i, 0, L"Downloading");
			isDling = true;	
		
			WaitForSingleObject(pi.hProcess, INFINITE);

			isDling = false;

			ExitOK = true;

			GetExitCodeProcess(pi.hProcess,&exitCode);

			if (exitCode == 0) {
				assumeError = false;
			}



			pwnd->SetDlgItemTextW(IDC_ETA, L"");
			pwnd->SetDlgItemTextW(IDC_STATUS, L"Idle");
			m_Prog->SetPos(100);
			
			if (assumeError == false) {
				m_FileQueue->SetItemText(i, 0, L"Done");
			} else { 
				m_FileQueue->SetItemText(i, 0, L"Error!");
			}
		}
	}

	queueRunning = false;

	bGetFile = (CButton*)pwnd->GetDlgItem(IDC_BUTTON3);
	bGetFile->EnableWindow(TRUE);

	return 1;
}

void CReimuGETDlg::OnBnClickedButton3()
{
	CWnd* pwnd = AfxGetMainWnd(); // Pointer to main window
	CWinThread* pDownloadFiles = AfxBeginThread(DownloadFiles,pwnd,THREAD_PRIORITY_NORMAL);
}


void CReimuGETDlg::OnLvnInsertitemList3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	//m_FileQueue.SortItems(SortItemURLs, 0);
}


BOOL CReimuGETDlg::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message == WM_KEYDOWN)) {
		if (pMsg->wParam == VK_RETURN) {
			CReimuGETDlg::OnBnClickedButton2();
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

UINT CheckLog(LPVOID pParam) {	
	CWnd* pwnd = AfxGetMainWnd(); // Pointer to main window
	HWND hWnd = pwnd->GetSafeHwnd();

	char cBuffer[BUFSIZE];
	char *cLinePos, *cLine;
	CString currLine;

	char * cToken;
	
	CString Downloaded, Total, Percent, Speed, ETA, Connections;
	char *downloaded, *total, *percent, *speed, *eta, *connections;
	int Progress = 0;

	CProgressCtrl* m_Prog = (CProgressCtrl*)pwnd->GetDlgItem(IDC_PROGRESS1);

	while(1) { // Yes, I should only leave this running when it is needed, but I'm lazy.
		if (isDling == true) {
			DWORD dwRead; 
			BOOL bSuccess = FALSE;
			HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

			for (;;) { 
				ZeroMemory(&cBuffer,BUFSIZE);

				bSuccess = ReadFile( g_hChildStd_OUT_Rd, cBuffer, BUFSIZE, &dwRead, NULL);
				if( ! bSuccess || dwRead == 0 ) break; 
			   
				for (cLine = strtok_s(cBuffer, "\r\n", &cLinePos); cLine; cLine = strtok_s(NULL, "\r\n", &cLinePos)) {
					if (cLine[0] == '[' && cLine[1] == '#') {
						cToken = strtok (cLine,":");
					
						downloaded = strtok(NULL,"/");
						if (downloaded != NULL) {
							Downloaded = CString(downloaded);
						}
					
						total = strtok(NULL,"(");
						if (total != NULL) {
							Total = CString(total);
						}

						percent = strtok(NULL,"%");
						if (percent != NULL) {
							Percent = CString(percent);
							Progress = _wtoi(Percent);
						}
						
						strtok(NULL,":");

						connections = strtok(NULL," ");
						if (connections != NULL) {
							Connections = CString(connections);
						}

						strtok(NULL,":");
						
						speed = strtok(NULL," ");
						if (speed != NULL) {
							Speed = CString(speed);
						}

						strtok(NULL,":");
						
						eta = strtok(NULL,"]");
						if (eta != NULL) {
							ETA = CString(eta);
						}
					}
				 }

				m_Prog->SetPos(Progress);
				
				pwnd->SetDlgItemTextW(IDC_STATUS, CString(L"[" + Percent + L"%] Downloaded " + Downloaded + L"/" + Total + L" at " + Speed + L" with " + Connections + L" connection(s)."));
				pwnd->SetDlgItemTextW(IDC_ETA, CString(L"Time remaining: " + ETA));
			} 
		} 
		
		Sleep(1000);
	}

	return 1;
}

void CReimuGETDlg::OnClose()
{
	CDialogEx::OnClose();
}

void CReimuGETDlg::OnCancel()
{
	bool doQuitNow = false;

	if (queueRunning) {
		int doQuit = AfxMessageBox(L"Are you sure you want to quit?", MB_YESNO|MB_ICONQUESTION);
		if (doQuit == IDYES) {
			doQuitNow = true;
		}
	} else {
		doQuitNow = true;
	}

	if (doQuitNow == true) {
		ExitOK = false;
		killAria = true;

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		CreateProcess(NULL, CString(L"taskkill.exe /F /IM ReimuGET_temp_aria2c.exe").GetBuffer(), NULL, NULL, false, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, INFINITE);

		CreateProcess(NULL, CString(L"cmd.exe /c del ReimuGET_temp_aria2c.exe").GetBuffer(), NULL, NULL, false, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

		CDialogEx::OnCancel();
	}
}
