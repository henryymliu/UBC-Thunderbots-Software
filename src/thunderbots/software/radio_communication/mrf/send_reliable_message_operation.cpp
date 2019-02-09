#include "send_reliable_message_operation.h"

#include "constants.h"

namespace
{
    std::unique_ptr<USB::BulkOutTransfer> create_reliable_message_transfer(
        USB::DeviceHandle &device, unsigned int robot, uint8_t message_id,
        unsigned int tries, const void *data, std::size_t length)
    {
        assert(robot < 8);
        assert((1 <= tries) && (tries <= 256));
        uint8_t buffer[3 + length];
        buffer[0] = static_cast<uint8_t>(robot | 0x10);
        buffer[1] = message_id;
        buffer[2] = static_cast<uint8_t>(tries & 0xFF);
        std::memcpy(buffer + 3, data, length);
        std::unique_ptr<USB::BulkOutTransfer> ptr(
            new USB::BulkOutTransfer(device, 3, buffer, sizeof(buffer), 64, 0));
        return ptr;
    }
}

SendReliableMessageOperation::SendReliableMessageOperation(MRFDongle &dongle,
                                                           unsigned int robot,
                                                           unsigned int tries,
                                                           const void *data,
                                                           std::size_t length)
    : dongle(dongle),
      message_id(dongle.alloc_message_id()),
      delivery_status(0xFF),
      transfer(create_reliable_message_transfer(dongle.device, robot, message_id, tries,
                                                data, length))
{
    transfer->signal_done.connect(
        sigc::mem_fun(this, &SendReliableMessageOperation::out_transfer_done));
    transfer->submit();
    mdr_connection = dongle.signal_message_delivery_report.connect(
        sigc::mem_fun(this, &SendReliableMessageOperation::message_delivery_report));
}

void SendReliableMessageOperation::result() const
{
    transfer->result();
    switch (delivery_status)
    {
        case MRF::MDR_STATUS_OK:
            break;
        case MRF::MDR_STATUS_NOT_ASSOCIATED:
            throw NotAssociatedError();
        case MRF::MDR_STATUS_NOT_ACKNOWLEDGED:
            throw NotAcknowledgedError();
        case MRF::MDR_STATUS_NO_CLEAR_CHANNEL:
            throw ClearChannelError();
        default:
            throw std::logic_error("Unknown delivery status");
    }
}

void SendReliableMessageOperation::out_transfer_done(AsyncOperation &op)
{
    if (!op.succeeded())
    {
        signal_done.emit(*this);
    }
}

void SendReliableMessageOperation::message_delivery_report(uint8_t id, uint8_t code)
{
    if (id == message_id)
    {
        mdr_connection.disconnect();
        delivery_status = code;
        dongle.free_message_id(message_id);
        signal_done.emit(*this);
    }
}

SendReliableMessageOperation::NotAssociatedError::NotAssociatedError()
    : std::runtime_error("Message sent to robot that is not associated")
{
}

SendReliableMessageOperation::NotAcknowledgedError::NotAcknowledgedError()
    : std::runtime_error("Message sent to robot not acknowledged")
{
}

SendReliableMessageOperation::ClearChannelError::ClearChannelError()
    : std::runtime_error("Message sent to robot failed to find clear channel")
{
}