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
  std::string name;
  Publisher* parent;

  Channel(SampledValuesPublisher publisher, std::string name, Publisher* parent):
    name(name),
    parent(parent),
    _asdu(SampledValuesPublisher_addASDU(publisher, (char *) name.c_str(), NULL, 1)) {}

  Value create_float_value() {
    Value value;
    value.type = ValueType::FLOAT;
    value.id = SV_ASDU_addFLOAT(_asdu);
    return value;
  }

  Value create_int_value() {
    Value value;
    value.type = ValueType::INT;
    value.id = SV_ASDU_addINT32(_asdu);
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
  std::string interface;
  std::vector<Channel> channels;

  Publisher(std::string interface):
    interface(interface),
    channels{},
    _publisher(SampledValuesPublisher_create(NULL, interface.c_str())) {}

  Channel* add_channel(std::string name) {
    Channel channel{_publisher, name, this};
    channels.push_back(channel);
    return &channels.back();
  }

  void broadcast() {
    for (auto &channel : channels) {
      channel.increment_sample_count();
    }
    SampledValuesPublisher_publish(_publisher);
  }

  void setup_complete() {
    SampledValuesPublisher_setupComplete(_publisher);
  }

private:
  SampledValuesPublisher _publisher;
};

#endif // _PUBLISHER_HPP_
