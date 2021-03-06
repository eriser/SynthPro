#include "pushbutton.h"

PushButton::PushButton(QObject* parent)
    : QObject(parent)
    , m_pushed(false)
    , m_enabled(true)
    , m_checkable(false)
    , m_checked(false)
{
}

void PushButton::push()
{
    m_pushed = true;
    if (m_checkable) {
        setChecked(!m_checked);
    }

    emit buttonPushed();
}

void PushButton::release()
{
    m_pushed = false;
    emit buttonReleased();
}

void PushButton::setEnabled(bool enabled)
{
    if (enabled != m_enabled) {
        m_enabled = enabled;
        emit buttonEnabled(enabled);
    }
}

void PushButton::setCheckable(bool checkable)
{
    if (checkable != m_checkable) {
        m_checkable = checkable;
        emit buttonCheckable(checkable);
    }
}

void PushButton::setChecked(bool checked)
{
    if (!m_checkable) {
        // Do nothing if the button isn't checkable.
        return;
    }

    if (checked != m_checked) {
        m_checked = checked;
        emit buttonChecked(m_checked);
    }
}
