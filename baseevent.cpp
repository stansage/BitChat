#include "baseevent.hpp"

using bitchat::BaseEvent;

int BaseEvent::m_sequence{ 0 };

BaseEvent::BaseEvent() : BaseEvent{ m_sequence++ }
{
}

BaseEvent::BaseEvent( const int tag ) :
    m_tag{ tag }
{
}

int BaseEvent::getTag() const
{
    return m_tag;
}
