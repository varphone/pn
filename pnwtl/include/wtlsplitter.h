/**
 * @file wtlsplitter.h
 * @author Simon Steele <mailto:ss@pnotepad.org>
 * @brief Simple splitter framework for WTL
 *
 * This code was inspired by sample code from James Brown:
 *		- http://freespace.virgin.net/james.brown7/tuts/splitter.htm
 */
#ifndef wtlsplitter_h__included
#define wtlsplitter_h__included

#define SPLITTER_NORMAL -1
#define SPLITTER_PANE1	0
#define SPLITTER_PANE2	1

#define SPLITTER_LEFT	SPLITTER_PANE1
#define SPLITTER_RIGHT	SPLITTER_PANE2
#define SPLITTER_TOP	SPLITTER_PANE1
#define SPLITTER_BOTTOM	SPLITTER_PANE2

template <class T>
class CWTLSplitter : public CWindowImpl<CWTLSplitter>
{
	public:
		CWTLSplitter()
		{
			m_oldY = -4;
			m_bMoved = false;
			m_bDragging = false;
			m_bHorz = false;
			
			SetSplitterSize(4);
			m_singlePane = SPLITTER_NORMAL;

			GetSysSettings();
		}

        BEGIN_MSG_MAP(CWTLSplitter)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		END_MSG_MAP()

		void SetPanes(HWND pane1, HWND pane2, bool bUpdate = true)
		{
			panes[0] = pane1;
			panes[1] = pane2;
			
			if(bUpdate)
				UpdateLayout();
		}

		void SetHorizontal(bool bHorizontal, bool bUpdate = false)
		{
			m_bHorz = bHorizontal;
			
			if(bUpdate)
			{
				CRect rc;
				::GetClientRect(m_hwOwner, rc);
				//We assume this is being changed mid-run, so we guess at the split position.
				if(m_bHorz)
				{
					//We were vertical...
					
					float ratio = (float)m_nSplitterPos / rc.Width();
					m_nSplitterPos = (int)(ratio * rc.Height());
				}
				else
				{
					float ratio = (float)m_nSplitterPos / rc.Height();
					m_nSplitterPos = (int)(ratio * rc.Width());
				}

				UpdateLayout();
			}
		}

		bool GetHorizontal()
		{
			return m_bHorz;
		}

		void SetSplitterSize(int size)
		{
			m_splitterSize = size;
			m_halfSize = size / 2;
		}

		int GetSplitterSize()
		{
			return m_splitterSize;
		}

		void SetSinglePaneMode(int pane, bool bUpdate = true)
		{
			m_singlePane = pane;
			
			if(bUpdate)
				UpdateLayout();
		}

		int GetSinglePaneMode()
		{
			return m_singlePane;
		}
		
		void DisableSinglePaneMode(bool bUpdate = true)
		{
			m_singlePane = SPLITTER_NORMAL;

			if(bUpdate)
				UpdateLayout();
		}

		void UpdateLayout()
		{
			T* pT = static_cast<T*>(this);
			pT->LayoutWindows();
		}

		void CentreSplit()
		{
			CRect rc;
			::GetClientRect(m_hwOwner, rc);

			if(m_bHorz)
			{
				m_nSplitterPos = rc.top + (rc.Height() / 2) - (m_splitterSize / 2);
			}
			else
			{
				m_nSplitterPos = rc.left + (rc.Width() / 2) - (m_splitterSize / 2);
			}
		}
		
	protected:
		void LayoutWindows()
		{
			CRect rc;
			::GetClientRect(m_hwOwner, rc);

			if(SPLITTER_NORMAL != m_singlePane)
			{
				::SetWindowPos(panes[m_singlePane], HWND_TOP, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
				MoveWindow(-1, -1, 0, 0);
				return;
			}

			CRect rc2(rc);

			if(m_bHorz)
				rc2.bottom = m_nSplitterPos - m_halfSize;
			else
				rc2.right = m_nSplitterPos - m_halfSize;
			
			::MoveWindow(panes[0], rc2.left, rc2.top, rc2.Width(), rc2.Height(), true);
			
			if(m_bHorz)
			{
				rc.top = rc2.bottom;
				rc2.top = rc2.bottom + m_splitterSize;
				rc2.bottom = rc.bottom;
				rc.bottom = rc2.top;
			}
			else
			{
				rc.left = rc2.right;
				rc2.left = rc2.right + m_splitterSize;
				rc2.right = rc.right;
				rc.right = rc2.left;
			}

			::MoveWindow(panes[1], rc2.left, rc2.top, rc2.Width(), rc2.Height(), true);
			MoveWindow(rc);
		}

		void DrawXorBar(HDC hdc, int x1, int y1, int width, int height)
		{
			static WORD _dotPatternBmp[8] = 
			{ 
				0x00aa, 0x0055, 0x00aa, 0x0055, 
				0x00aa, 0x0055, 0x00aa, 0x0055
			};

			HBITMAP hbm;
			HBRUSH  hbr, hbrushOld;

			hbm = CreateBitmap(8, 8, 1, 1, _dotPatternBmp);
			hbr = CreatePatternBrush(hbm);
			
			SetBrushOrgEx(hdc, x1, y1, 0);
			hbrushOld = (HBRUSH)SelectObject(hdc, hbr);
			
			PatBlt(hdc, x1, y1, width, height, PATINVERT);
			
			SelectObject(hdc, hbrushOld);
			
			DeleteObject(hbr);
			DeleteObject(hbm);
		}

		void GetSysSettings()
		{
			BOOL b;
			::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &b, 0);
			m_bFullDrag = b != 0;
		}

	// Message Handlers:
	protected:
		void InternalAdjustPoints(LPRECT lpRect, LPPOINT lpPoint)
		{
			RECT wrect;

			::GetWindowRect(m_hwOwner, &wrect);
			::GetClientRect(m_hwOwner, lpRect);
			::ClientToScreen(m_hwOwner, (LPPOINT)lpRect);
			::ClientToScreen(m_hwOwner, ((LPPOINT)lpRect)+1);

			//convert the mouse coordinates relative to the top-left of the window
			ClientToScreen(lpPoint);
			lpPoint->x -= wrect.left;
			lpPoint->y -= wrect.top;

			//same for the window coordinates - make them relative to 0,0
			::OffsetRect(lpRect, -wrect.left, -wrect.top);

			if(m_bHorz)
			{
				if(lpPoint->y < lpRect->top)
					lpPoint->y = lpRect->top;

				if(lpPoint->y > lpRect->bottom - m_splitterSize)
					lpPoint->y = lpRect->bottom - m_splitterSize;
			}
			else
			{
				if(lpPoint->x < lpRect->left)
					lpPoint->x = lpRect->left;

				if(lpPoint->x > lpRect->right - m_splitterSize)
					lpPoint->x = lpRect->right - m_splitterSize;
			}
		}

		void InternalReAdjustPoint(LPPOINT lpPoint)
		{
			RECT rect;

			::GetWindowRect(m_hwOwner, &rect);
			lpPoint->x += rect.left;
			lpPoint->y += rect.top;

			//now convert into CLIENT coordinates
			::ScreenToClient(m_hwOwner, lpPoint);
		}

		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
		{
			m_hwOwner = GetParent();
			return 0;
		}

		LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
		{
			POINT pt;
			HDC hdc;
			RECT rect;

			pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
			pt.y = (short)HIWORD(lParam);
			
			(static_cast<T*>(this))->InternalAdjustPoints(&rect, &pt);

			m_bDragging = true;

			//::SetCapture(m_hwOwner);
			SetCapture();

			if(!m_bFullDrag)
			{
				hdc = ::GetWindowDC(m_hwOwner);
				if(m_bHorz)
				{
					DrawXorBar(hdc, rect.left + 1, pt.y - m_halfSize, rect.right - 2, m_splitterSize);
					m_oldY = pt.y;
				}
				else
				{
					DrawXorBar(hdc, pt.x - m_halfSize, rect.top+1, m_splitterSize, rect.bottom - 2);
					m_oldY = pt.x;
				}
				::ReleaseDC(m_hwOwner, hdc);
			}
			
			return 0;
		}

		LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
		{
			if((!m_bDragging))
				return 0;

			if(m_bFullDrag)
			{
				m_bDragging = false;
				ReleaseCapture();
				return 0;
			}

			HDC hdc;
			RECT rect;

			POINT pt;
			pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
			pt.y = (short)HIWORD(lParam);
			
			(static_cast<T*>(this))->InternalAdjustPoints(&rect, &pt);

			hdc = ::GetWindowDC(m_hwOwner);
			if(m_bHorz)
			{
				DrawXorBar(hdc, rect.left+1, m_oldY - m_halfSize, rect.right - 2, m_splitterSize);
				m_oldY = pt.y;
			}
			else
			{
				DrawXorBar(hdc, m_oldY - m_halfSize, rect.top+1, m_splitterSize, rect.bottom-2);
				m_oldY = pt.x;
			}
			::ReleaseDC(m_hwOwner, hdc);

			m_bDragging = false;

			(static_cast<T*>(this))->InternalReAdjustPoint(&pt);
			
			if(m_bHorz)
				m_nSplitterPos = pt.y;
			else
				m_nSplitterPos = pt.x;
			
			UpdateLayout();

			::ReleaseCapture();

			return 0;
		}

		LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
		{
			if(!m_bDragging)
				return 0;

			HDC hdc;
			RECT rect;

			POINT pt;

			pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
			pt.y = (short)HIWORD(lParam);

			(static_cast<T*>(this))->InternalAdjustPoints(&rect, &pt);

			if(m_bHorz)
			{
				if(pt.y != m_oldY && ((wParam & MK_LBUTTON) != 0))
				{
					if(!m_bFullDrag)
					{
						hdc = ::GetWindowDC(m_hwOwner);
						DrawXorBar(hdc, rect.left+1, m_oldY - m_halfSize, rect.right - 2, m_splitterSize);
						DrawXorBar(hdc, rect.left+1, pt.y - m_halfSize, rect.right - 2, m_splitterSize);
							
						::ReleaseDC(m_hwOwner, hdc);

						m_oldY = pt.y;
					}
					else
					{
						//convert the splitter position back to screen coords.
						(static_cast<T*>(this))->InternalReAdjustPoint(&pt);
						m_nSplitterPos = pt.y;
						UpdateLayout();
					}
				}
			}
			else
			{
				if(pt.x != m_oldY && ((wParam & MK_LBUTTON) != 0))
				{
					if(!m_bFullDrag)
					{
						hdc = ::GetWindowDC(m_hwOwner);
						DrawXorBar(hdc, m_oldY - m_halfSize, rect.top+1, m_splitterSize, rect.bottom - 2);
						DrawXorBar(hdc, pt.x - m_halfSize, rect.top+1, m_splitterSize, rect.bottom - 2);
						::ReleaseDC(m_hwOwner, hdc);

						m_oldY = pt.x;
					}
					else
					{
						(static_cast<T*>(this))->InternalReAdjustPoint(&pt);
						m_nSplitterPos = pt.x;
						UpdateLayout();
					}
				}
			}

			return 0;
		}

		LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
		{
			CPaintDC dc(m_hWnd);

			CRect rect;
			GetClientRect(rect);

			dc.FillRect(&rect, COLOR_3DFACE);

			/* WS_EX_CLIENTEDGE stops the class working... Code disabled to discourage this...
			if((GetExStyle() & WS_EX_CLIENTEDGE) != 0)
				dc.DrawEdge(&rect, EDGE_RAISED, m_bHorz ? (BF_TOP | BF_BOTTOM) : (BF_LEFT | BF_RIGHT));*/
			
			return 0;
		}

		LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
		{
			GetSysSettings();
			UpdateLayout();
			return 0;
		}

	protected:
        HWND	m_hwOwner;
		HWND	panes[2];
		bool	bSinglePane;

		int		m_singlePane;
		int		m_halfSize;
		int		m_splitterSize;
		int		m_nSplitterPos;
		int		m_oldY;
		bool	m_bMoved;
		bool	m_bDragging;
		bool	m_bHorz;
		bool	m_bFullDrag;
};

class CSimpleSplitter : public CWTLSplitter<CSimpleSplitter> {};

#endif