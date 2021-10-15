#include "framelessmainwindow.h"

#include <QDebug>
#include <QEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#pragma comment (lib, "user32.lib")
#endif

using namespace vnotex;

FramelessMainWindow::FramelessMainWindow(bool p_frameless, QWidget *p_parent)
    : QMainWindow(p_parent),
      m_frameless(p_frameless),
      m_defaultFlags(windowFlags())
{
    if (m_frameless) {
        m_resizeAreaWidth *= devicePixelRatio();

        setWindowFlags(m_defaultFlags | Qt::FramelessWindowHint);

        // Enable some window effects on Win, such as snap and maximizing.
        // It will activate the title bar again. Need to remove it in WM_NCCALCSIZE msg.
#ifdef Q_OS_WIN
        HWND hwnd = (HWND)this->winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
#endif
    }
}

void FramelessMainWindow::showEvent(QShowEvent *p_event)
{
    // Force update after show.
    if (m_frameless) {
        setAttribute(Qt::WA_Mapped);
    }
    QMainWindow::showEvent(p_event);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
bool FramelessMainWindow::nativeEvent(const QByteArray &p_eventType, void *p_message, qintptr *p_result)
#else
bool FramelessMainWindow::nativeEvent(const QByteArray &p_eventType, void *p_message, long *p_result)
#endif
{
    if (!m_frameless) {
        return QMainWindow::nativeEvent(p_eventType, p_message, p_result);
    }

#ifdef Q_OS_WIN
    if (p_eventType == QStringLiteral("windows_generic_MSG")) {
        MSG *msg = static_cast<MSG *>(p_message);

        if (msg->message == WM_NCCALCSIZE) {
            *p_result = 0;
            return true;
        } else if (msg->message == WM_NCHITTEST) {
            // Get the screen position of mouse.
            // https://docs.microsoft.com/zh-cn/windows/win32/inputdev/wm-nchittest
            RECT windowRect;
            ::GetWindowRect(msg->hwnd, &windowRect);

            // x and y could not be compared with width() and height() in hidpi case.
            int x = static_cast<int>(GET_X_LPARAM(msg->lParam) - windowRect.left);
            int y = static_cast<int>(GET_Y_LPARAM(msg->lParam) - windowRect.top);

            bool onLeft = x < m_resizeAreaWidth;
            bool onRight = x > windowRect.right - windowRect.left - m_resizeAreaWidth;
            bool onTop = y < m_resizeAreaWidth;
            bool onBottom = y > windowRect.bottom - windowRect.top - m_resizeAreaWidth;

            *p_result = 0;
            if (m_resizable) {
                if (onLeft && onTop) {
                    *p_result = HTTOPLEFT;
                } else if (onLeft && onBottom) {
                    *p_result = HTBOTTOMLEFT;
                } else if (onRight && onTop) {
                    *p_result = HTTOPRIGHT;
                } else if (onRight && onBottom) {
                    *p_result = HTBOTTOMRIGHT;
                } else if (onLeft) {
                    *p_result = HTLEFT;
                } else if (onRight) {
                    *p_result = HTRIGHT;
                } else if (onTop) {
                    *p_result = HTTOP;
                } else if (onBottom) {
                    *p_result = HTBOTTOM;
                }
            }

            if (0 != *p_result) {
                return true;
            }

            if (m_titleBar != 0 && y < m_titleBarHeight) {
                QWidget *child = m_titleBar->childAt(m_titleBar->mapFromGlobal(QCursor::pos()));
                if (!child) {
                    *p_result = HTCAPTION;
                    return true;
                }
            }
        } else if (msg->wParam == PBT_APMSUSPEND && msg->message == WM_POWERBROADCAST) {
            // Minimize when system is going to sleep to avoid bugs.
            showMinimized();
        } else if (msg->wParam == PBT_APMRESUMEAUTOMATIC) {
            // Show after resuming from sleep.
            showNormal();
        }
    }
#endif

    return QMainWindow::nativeEvent(p_eventType, p_message, p_result);
}

bool FramelessMainWindow::eventFilter(QObject *p_obj, QEvent *p_event)
{
    return QMainWindow::eventFilter(p_obj, p_event);
}

bool FramelessMainWindow::isFrameless() const
{
    return m_frameless;
}

void FramelessMainWindow::setTitleBar(QWidget *p_titleBar)
{
    Q_ASSERT(!m_titleBar && m_frameless);

    m_titleBar = p_titleBar;
    m_titleBarHeight = m_titleBar->height() * devicePixelRatio();
    m_titleBar->installEventFilter(this);
}

void FramelessMainWindow::changeEvent(QEvent *p_event)
{
    QMainWindow::changeEvent(p_event);

    if (p_event->type() == QEvent::WindowStateChange) {
        updateMargins();
        emit windowStateChanged(windowState());
    }
}

void FramelessMainWindow::updateMargins()
{
    if (!m_frameless) {
        return;
    }

    int topMargin = m_extraTopMargin;
    const auto state = windowState();
    if ((state & Qt::WindowMaximized) && !(state & Qt::WindowFullScreen)) {
        topMargin += m_marginOnMaximized;
        setContentsMargins(m_marginOnMaximized, topMargin, m_marginOnMaximized, m_marginOnMaximized);
    } else {
        topMargin += m_margin;
        setContentsMargins(m_margin, topMargin, m_margin, m_margin);
    }

    m_titleBarHeight = (m_titleBar->height() + topMargin) * devicePixelRatio();
}
