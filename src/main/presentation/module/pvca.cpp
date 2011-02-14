#include "pvca.h"

#include "control/module/cvca.h"
#include "presentation/component/pdimmer.h"
#include "presentation/component/pvirtualport.h"
#include "presentation/widget/textwidget.h"

#include <QFont>
#include <QGraphicsAnchorLayout>

PVCA::PVCA(CVCA* control)
    : PModule(control)
{
}

void PVCA::initialize(PVirtualPort* in, PVirtualPort* out, PVirtualPort* controlInput, PDimmer* gain)
{
    TextWidget* title = new TextWidget("VCA", this);
    title->setFont(QFont("Courier", 18, QFont::Bold));

    gain->setMaximumSize(90, 90);

    // Layout
    bottomArea()->addCornerAnchors(controlInput, Qt::BottomLeftCorner, bottomArea(), Qt::BottomLeftCorner);
    bottomArea()->addAnchor(controlInput, Qt::AnchorRight, gain, Qt::AnchorLeft);
    bottomArea()->addAnchor(gain, Qt::AnchorRight, bottomArea(), Qt::AnchorRight);
    bottomArea()->addAnchors(gain, bottomArea(), Qt::Vertical);
    leftArea()->addAnchors(in, leftArea());
    rightArea()->addAnchors(out, rightArea());
    centerArea()->addAnchors(title, centerArea());
}