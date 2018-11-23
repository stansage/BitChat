#pragma once

#include <boost/noncopyable.hpp>
#include <functional>

namespace bitchat {

class BaseEvent
{
public:
    using Target = std::function<bool (void*)>;

protected:
    BaseEvent();
    explicit BaseEvent( const int tag );

public:
    int getTag() const;

protected:
    int m_tag;
    static int m_sequence;

};

} // bitchat
