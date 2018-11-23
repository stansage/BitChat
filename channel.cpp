#include "channel.hpp"
#include "filechannel.hpp"
#include "socketchannel.hpp"
#include "communication.hpp"
#include <boost/log/trivial.hpp>

using bitchat::Channel;

const Channel::Event Channel::kOnOpen{};
const Channel::Event Channel::kOnClose{};

Channel::Channel()
{
    BOOST_LOG_TRIVIAL( debug ) << "Created channel - " << this;
}

Channel::~Channel()
{
    BOOST_LOG_TRIVIAL( debug ) << "Destroyed channel - " << this;
}
