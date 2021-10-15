#ifndef FRAMELESSMAINWINDOW_H
#define FRAMELESSMAINWINDOW_H

#include <QMainWindow>

namespace vnotex
{
    // From https://github.com/feiyangqingyun/QWidgetDemo.
    class FramelessMainWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        FramelessMainWindow(bool p_frameless = true, QWidget *p_parent = nullptr);

        bool isFrameless() const;

        void setTitleBar(QWidget *p_titleBar);

    signals:
        void windowStateChanged(Qt::WindowStates p_state);

    protected:
        void showEvent(QShowEvent *p_event);

        bool eventFilter(QObject *p_obj, QEvent *p_event);

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        bool nativeEvent(const QByteArray &p_eventType, void *p_message, qintptr *p_result);
#else
        bool nativeEvent(const QByteArray &p_eventType, void *p_message, long *p_result);
#endif

        void changeEvent(QEvent *p_event) Q_DECL_OVERRIDE;

    private:
        void updateMargins();

        const bool m_frameless = true;

        int m_resizeAreaWidth = 5;

        int m_margin = 1;

        // On Windows, when maximized, we need some extra margins.
        int m_marginOnMaximized = 5;

        int m_extraTopMargin = 2;

        bool m_movable = true;

        bool m_resizable = true;

        const Qt::WindowFlags m_defaultFlags;

        QWidget *m_titleBar = nullptr;

        int m_titleBarHeight = 0;
    };
}

#endif // FRAMELESSMAINWINDOW_H
