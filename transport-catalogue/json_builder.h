#pragma once
#include "json.h"
#include <optional>

namespace json {

    class Builder {
    private:
        class BaseContext;
        class DictItemContext;
        class ArrayItemContext;
        class KeyContext;

    public:
        Builder()
            : root_(nullptr) {
        }

        KeyContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();

    private:
        class BaseContext {
        public:
            BaseContext(Builder& builder)
                : builder_(builder) {
            }

        protected:
            Builder& builder_;
        };

        class DictItemContext : public BaseContext {
        public:
            using BaseContext::BaseContext;

            KeyContext Key(std::string key);
            Builder& EndDict();

            DictItemContext Value(Node::Value) = delete;
            Builder& EndArray() = delete;
            ArrayItemContext StartArray() = delete;
            DictItemContext StartDict() = delete;
        };

        class ArrayItemContext : public BaseContext {
        public:
            using BaseContext::BaseContext;

            ArrayItemContext Value(Node::Value value);
            DictItemContext StartDict();
            ArrayItemContext StartArray();
            Builder& EndArray();

            KeyContext Key(std::string) = delete;
            Builder& EndDict() = delete;
        };

        class KeyContext : public BaseContext {
        public:
            using BaseContext::BaseContext;

            DictItemContext Value(Node::Value value);
            DictItemContext StartDict();
            ArrayItemContext StartArray();

            KeyContext Key(std::string) = delete;
            Builder& EndDict() = delete;
            Builder& EndArray() = delete;
        };

    private:
        Node root_;
        std::vector<Node*> nodes_stack_{};
        std::optional<std::string> key_;
    };

}