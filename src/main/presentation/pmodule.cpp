#include "pmodule.h"

#include "control/cmodule.h"
#include "presentation/widget/pixmapbuttonwidget.h"
#include "presentation/widget/textwidget.h"
#include <QApplication>
#include <QBrush>
#include <QGraphicsAnchorLayout>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyle>

PModule::PModule(CModule* control)
    : QGraphicsWidget(0)
    , m_control(control)
    , m_leftArea(new QGraphicsAnchorLayout)
    , m_bottomArea(new QGraphicsAnchorLayout)
    , m_centerArea(new QGraphicsAnchorLayout)
    , m_rightArea(new QGraphicsAnchorLayout)
{
    setFlag(ItemIsMovable);

    // Setup Layout
    QGraphicsAnchorLayout* layout = new QGraphicsAnchorLayout(this);
    layout->setSpacing(0);

    m_leftArea->setParentLayoutItem(layout);
    m_leftArea->setSpacing(0);
    m_centerArea->setParentLayoutItem(layout);
    m_centerArea->setSpacing(0);
    m_bottomArea->setParentLayoutItem(layout);
    m_bottomArea->setSpacing(0);
    m_rightArea->setParentLayoutItem(layout);
    m_rightArea->setSpacing(0);

    layout->addAnchor(m_leftArea, Qt::AnchorRight, m_centerArea, Qt::AnchorLeft);
    layout->addAnchor(m_bottomArea, Qt::AnchorTop, m_centerArea, Qt::AnchorBottom);
    layout->addAnchor(m_rightArea, Qt::AnchorLeft, m_centerArea, Qt::AnchorRight);
    layout->addAnchor(m_centerArea, Qt::AnchorTop, layout, Qt::AnchorTop);
    layout->addCornerAnchors(m_leftArea, Qt::TopLeftCorner, layout, Qt::TopLeftCorner);
    layout->addCornerAnchors(m_bottomArea, Qt::BottomLeftCorner, layout, Qt::BottomLeftCorner);
    layout->addCornerAnchors(m_bottomArea, Qt::BottomRightCorner, layout, Qt::BottomRightCorner);
    layout->addCornerAnchors(m_rightArea, Qt::TopRightCorner, layout, Qt::TopRightCorner);
    layout->addCornerAnchors(m_leftArea, Qt::BottomLeftCorner, m_bottomArea, Qt::TopLeftCorner);
    layout->addCornerAnchors(m_rightArea, Qt::BottomRightCorner, m_bottomArea, Qt::TopRightCorner);

    // Add a shiny shadow.
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setOffset(3);
    shadow->setBlurRadius(9);
    setGraphicsEffect(shadow);

    PixmapButtonWidget* closeBtn = new PixmapButtonWidget(":/src/resources/images/close-icon.png", this);
    closeBtn->setPos(-16, -16);
    connect(closeBtn, SIGNAL(clicked()), SIGNAL(closeBtnClicked()));
}

void PModule::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QStyle* style = QApplication::style();
    painter->setPen(Qt::NoPen);
    painter->setBrush(style->standardPalette().brush(QPalette::Window));

    painter->drawRect(rect());
}

void PModule::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    m_control->move();
}
