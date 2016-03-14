// Copyright 2016 Connor Taffe

#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_

#include <memory>
#include <set>
#include <string>

namespace basilisk {

class Event {
 public:
  virtual ~Event();
  virtual std::string description() = 0;
};

class Publisher {
 public:
  virtual ~Publisher();
  virtual void publish(std::shared_ptr<Event> e) = 0;
};

class Subscriber {
 public:
  virtual ~Subscriber();

  virtual void subscribe(std::shared_ptr<Publisher> handler) = 0;
  virtual void unsubscribe(std::shared_ptr<Publisher> handler) = 0;
};

class PublishSubscriber : public Publisher, Subscriber {
 public:
  PublishSubscriber();
  virtual ~PublishSubscriber();
};

class Topic : public PublishSubscriber {
 public:
  virtual ~Topic();
  void publish(std::shared_ptr<Event> e) final;
  void subscribe(std::shared_ptr<Publisher> handler) final;
  void unsubscribe(std::shared_ptr<Publisher> handler) final;

 protected:
  virtual bool filter(std::shared_ptr<Event> e) = 0;

 private:
  std::set<std::shared_ptr<Publisher>> handlers;
};

// Spool singleton allows global publishing
// to all subscribed publishers.
// Subscribed publishers allow filtering by providing
// thier own singleton pub-subs.
class Spool : public PublishSubscriber {
 public:
  static Spool *getInstance();
  void publish(std::shared_ptr<Event> e) final;
  void subscribe(std::shared_ptr<Publisher> handler) final;
  void unsubscribe(std::shared_ptr<Publisher> handler) final;

 private:
  Spool();
  Spool(const Spool &) = delete;
  static Spool *instance;
  std::set<std::shared_ptr<Publisher>> handlers;
};

}  // namespace basilisk

#endif  // SRC_EVENT_H_
