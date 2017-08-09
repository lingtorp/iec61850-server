#ifndef _PUBLISHER_HPP_
#define _PUBLISHER_HPP_

#include <vector>
#include <string>
#include <iostream>

/* libiec61850 */
#include "sv_publisher.h"

/* Forward declarations */
class Channel;
class Publisher;

/** All of the types a Value in a channel can have */
enum ValueType { INT, FLOAT };

/**  */
enum ValueConfig { MANUAL, SIN };

/**
 * Value
 * - Represents a variable inside a Channel in the API of libiec61850.
 */
struct Value {
  int id;
  ValueType type;
  ValueConfig config;
};

/**
 * Channel
 * - Channel wraps the API provided by libiec61850 when handling variables in a
 * channel.
 */
class Channel {
public:
  static const uint16_t MAX_NUM_VALUES = 16;
  std::vector<Value> values;
  /** Name of the channel (visible to the clients) */
  std::string name;

  Channel(SampledValuesPublisher publisher, std::string name):
    name(name),
    _asdu(SampledValuesPublisher_addASDU(publisher, (char *) name.c_str(), NULL, 1)),
    values{} {}

  /** Creates a Value (more like a variable) in the channel */
  Value create_float_value() {
    Value value;
    value.type = ValueType::FLOAT;
    value.id = SV_ASDU_addFLOAT(_asdu);
    values.push_back(value);
    return value;
  }

  /** Creates a Value (more like a variable) in the channel */
  Value create_int_value() {
    Value value;
    value.type = ValueType::INT;
    value.id = SV_ASDU_addINT32(_asdu);
    values.push_back(value);
    return value;
  }

  /** Sets the channel's Value (more like a variable) to val */
  void set_value(Value value, float val) {
    assert(value.type == ValueType::FLOAT);
    SV_ASDU_setFLOAT(_asdu, value.id, val);
  }

  /** Sets the channel's Value (more like a variable) to val */
  void set_value(Value value, int val) {
    assert(value.type == ValueType::INT);
    SV_ASDU_setINT32(_asdu, value.id, val);
  }

  /** Increments the sample count of the channel */
  void increment_sample_count() {
    SV_ASDU_increaseSmpCnt(_asdu);
  }

private:
  /** libiec61850 */
  SV_ASDU _asdu;
};

/**
 * Publisher
 * - Wraps the C API provided by libiec61850 and makes it easier to handle.
 * All of the Channels and Values must be set before broadcasting can begin.
 * This is done by creating Channels and then creating Values inside of the
 * Channels. When the configuration is done, call complete_setup. The Publisher
 * is now ready to start broadcasting, no further changes to the Channels and
 * Values is possible except for changing the value of the Values (set_value).
 */
class Publisher {
public:
  static const uint16_t MAX_NUM_CHANNELS = 16;
  std::string interface;
  std::vector<Channel> channels;
  bool running;
  bool setup_completed;

  Publisher(std::string interface):
    interface(interface),
    channels{},
    _publisher(SampledValuesPublisher_create(NULL, interface.c_str())),
    running(false),
    setup_completed(false) {}

  /** Appends a new Channel to the publishers list of channels */
  Channel* add_channel(std::string name) {
    if (channels.size() <= MAX_NUM_CHANNELS) {
      Channel channel{_publisher, name};
      channels.push_back(channel);
    }
    return &channels.back();
  }

  /** Sends the Channels values over the network */
  void broadcast() {
    if (!running) { return; }
    for (auto &channel : channels) {
      channel.increment_sample_count();
    }
    SampledValuesPublisher_publish(_publisher);
  }

  /** Needs to be called before running and after setting up the Channels and Values */
  void complete_setup() {
    setup_completed = true;
    SampledValuesPublisher_setupComplete(_publisher);
  }

private:
  /** libiec61850 */
  SampledValuesPublisher _publisher;
};

#endif // _PUBLISHER_HPP_
