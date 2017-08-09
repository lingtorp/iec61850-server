#ifndef _PUBLISHER_HPP_
#define _PUBLISHER_HPP_

#include <vector>
#include <string>
#include <iostream>

#include "sv_publisher.h"

/* Forward declarations */
class Channel;
class Publisher;

enum ValueType { INT, FLOAT };

struct Value {
  int id;
  ValueType type;
};

class Channel {
public:
  static const uint16_t MAX_NUM_VALUES = 16;
  std::vector<Value> values;
  std::string name;
  Publisher* parent;

  Channel(SampledValuesPublisher publisher, std::string name, Publisher* parent):
    name(name),
    parent(parent),
    _asdu(SampledValuesPublisher_addASDU(publisher, (char *) name.c_str(), NULL, 1)),
    values{} {}

  Value create_float_value() {
    Value value;
    value.type = ValueType::FLOAT;
    value.id = SV_ASDU_addFLOAT(_asdu);
    values.push_back(value);
    return value;
  }

  Value create_int_value() {
    Value value;
    value.type = ValueType::INT;
    value.id = SV_ASDU_addINT32(_asdu);
    values.push_back(value);
    return value;
  }

  void set_value(Value value, float val) {
    assert(value.type == ValueType::FLOAT);
    SV_ASDU_setFLOAT(_asdu, value.id, val);
  }

  void set_value(Value value, int val) {
    assert(value.type == ValueType::INT);
    SV_ASDU_setINT32(_asdu, value.id, val);
  }

  void increment_sample_count() {
    SV_ASDU_increaseSmpCnt(_asdu);
  }

private:
  SV_ASDU _asdu;
};

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

  Channel* add_channel(std::string name) {
    if (channels.size() <= MAX_NUM_CHANNELS) {
      Channel channel{_publisher, name, this};
      channels.push_back(channel);
    }
    return &channels.back();
  }

  void broadcast() {
    if (!running) { return; }
    for (auto &channel : channels) {
      channel.increment_sample_count();
    }
    SampledValuesPublisher_publish(_publisher);
  }

  void complete_setup() {
    setup_completed = true;
    SampledValuesPublisher_setupComplete(_publisher);
  }

private:
  SampledValuesPublisher _publisher;
};

#endif // _PUBLISHER_HPP_
