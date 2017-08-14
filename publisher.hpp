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

/** Dictactes what type of value manipulation is done on the value */
/** sine = system simulates a sine wave [-1.0f, 1.0f] */
/** MANUAL = system allows user to send whatever the is inputted in the GUI */
enum ValueConfig { MANUAL, sine };

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

  Channel() = delete;

  Channel(SampledValuesPublisher publisher, std::string a_name):
    name{a_name},
    values{} {
    str = new char[name.size() + 1]; // + 1 for null terminator
    std::memcpy(str, name.c_str(), name.size() + 1);
    _asdu = SampledValuesPublisher_addASDU(publisher, str, NULL, 1);
  }

  /** Creates a Value (more like a variable) in the channel */
  Value create_float_value() {
    Value value;
    value.type = ValueType::FLOAT;
    value.id = SV_ASDU_addFLOAT(_asdu);
    value.config = ValueConfig::MANUAL;
    values.push_back(value);
    return value;
  }

  /** Sets the channel's Value (more like a variable) to val */
  void set_value(Value value, float val) {
    assert(value.type == ValueType::FLOAT);
    SV_ASDU_setFLOAT(_asdu, value.id, val);
  }

  /** Increments the sample count of the channel */
  void increment_sample_count() {
    SV_ASDU_increaseSmpCnt(_asdu);
  }

private:
  /** libiec61850 */
  SV_ASDU _asdu;
  char* str; // libiec61850 owns this pointer (libiec61850 will dealloc it)
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

  Publisher() = delete;

  Publisher(std::string interface):
    interface(interface),
    channels{},
    running(false),
    setup_completed(false),
    _publisher(SampledValuesPublisher_create(NULL, interface.c_str())) {}

  ~Publisher() {
    /** Cleanup libiec61850 */
    SampledValuesPublisher_destroy(_publisher);
  }

  /** Appends a new Channel to the publishers list of channels */
  Channel* add_channel(std::string name) {
    if (channels.size() <= MAX_NUM_CHANNELS) {
      channels.emplace_back(Channel{_publisher, name});
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
