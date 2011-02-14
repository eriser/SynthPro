#include "poscilloscope.h"

#include "abstraction/buffer.h"
#include "control/module/coscilloscope.h"
#include "presentation/component/pvirtualport.h"
#include "presentation/widget/poscilloscopeview.h"
#include "presentation/widget/textwidget.h"

#include <QFont>
#include <QGraphicsAnchorLayout>
#include <QGraphicsSimpleTextItem>
#include <QTimer>

POscilloscope::POscilloscope(COscilloscope* control)
    : PModule(control)
    , m_refreshTimer(0)
    , m_mustRefreshOscilloscopeView(false)
{
}

void POscilloscope::initialize(PVirtualPort* input)
{
    TextWidget* title = new TextWidget("OSC", this);
    title->setFont(QFont("Courier", 18, QFont::Bold));

    m_pOscilloscopeView = new POscilloscopeView(this);

    // Layout
    leftArea()->addAnchors(input, leftArea());
    bottomArea()->addAnchors(m_pOscilloscopeView, bottomArea());
    centerArea()->addAnchors(title, centerArea());

    // Set up the refresh timer.
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->start(REFRESH_RATE);
    connect(m_refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTimerExpired()));

}

void POscilloscope::setVisualizedBuffer(Buffer* buffer)
{
    if (m_pOscilloscopeView) {
        m_pOscilloscopeView->setVisualizedBuffer(buffer);
    }
}

void POscilloscope::refreshOscilloscopeView()
{
    if (m_mustRefreshOscilloscopeView && m_pOscilloscopeView) {
        m_mustRefreshOscilloscopeView = false;
        m_pOscilloscopeView->update(0, 0, m_pOscilloscopeView->WIDTH, m_pOscilloscopeView->HEIGHT);
    }
}

void POscilloscope::refreshTimerExpired()
{
    m_mustRefreshOscilloscopeView = true;
}