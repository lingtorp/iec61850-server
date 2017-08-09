#ifndef _SERVER_HPP_
#define _SERVER_HPP_

class Server {
public:

  Server():
    running(false),
    setup_complete(false),
    publisher{} {}

  void setup_complete() {
    publisher.setup_complete();
    setup_complete = true;
  }

  void start(std::string interface) {
    publisher = Publisher{interface};
  }

  void stop() {

  }

  void broadcast() {
    publisher.broadcast();
  }

private:
  bool running;
  bool setup_complete;
  Publisher publisher;
};

#endif // _SERVER_HPP_
