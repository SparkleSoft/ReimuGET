
// ReimuGETDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CReimuGETDlg dialog
class CReimuGETDlg : public CDialogEx
{
// Construction
public:
	CReimuGETDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ReimuGET_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_FileQueue;
	afx_msg void OnBnClickedButton2();
	CEdit m_URL;
	CSliderCtrl m_Connections;
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton3();
	afx_msg void OnLvnInsertitemList3(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CProgressCtrl m_progBar;
//	afx_msg void OnClose();
//	virtual void OnCancel();
	afx_msg void OnClose();
	virtual void OnCancel();
};
