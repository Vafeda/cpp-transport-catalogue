#include "json_builder.h"

namespace json {

    Builder::KeyContext Builder::Key(std::string key) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("");
        }
        if (!nodes_stack_.back()->IsDict()) {
            throw std::logic_error("");
        }
        else if (key_.has_value()) {
            throw std::logic_error("");
        }

        key_ = std::move(key);
        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        Node v(value);

        if (nodes_stack_.empty()) {
            if (root_.IsNull()) {
                root_ = v;
            }
            else {
                throw std::logic_error("");
            }
        }
        else if (nodes_stack_.back()->IsDict()) {
            Dict& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
            if (!key_.has_value()) {
                throw std::logic_error("");
            }
            dict[key_.value()] = v;
            key_.reset();
        }
        else if (nodes_stack_.back()->IsArray()) {
            Array& array = const_cast<Array&>(nodes_stack_.back()->AsArray());
            array.push_back(v);
        }

        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        if (nodes_stack_.empty()) {
            root_ = Dict{};
            nodes_stack_.push_back(&root_);
        }
        else if (nodes_stack_.back()->IsArray()) {
            Array& array = const_cast<Array&>(nodes_stack_.back()->AsArray());
            array.push_back(Dict{});
            Node* new_node = &array.back();
            nodes_stack_.push_back(new_node);
        }
        else if (nodes_stack_.back()->IsDict()) {
            Dict& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
            if (!key_.has_value()) {
                throw std::logic_error("");
            }
            dict[key_.value()] = Dict{};
            Node* new_node = &dict[key_.value()];
            nodes_stack_.push_back(new_node);
            key_.reset();
        }

        return DictItemContext(*this);
    }

    Builder::ArrayItemContext Builder::StartArray() {
        if (nodes_stack_.empty()) {
            root_ = Array{};
            nodes_stack_.push_back(&root_);
        }
        else if (nodes_stack_.back()->IsArray()) {
            Array& array = const_cast<Array&>(nodes_stack_.back()->AsArray());
            array.push_back(Array{});
            Node* new_node = &array.back();
            nodes_stack_.push_back(new_node);
        }
        else if (nodes_stack_.back()->IsDict()) {
            Dict& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
            if (!key_.has_value()) {
                throw std::logic_error("");
            }
            dict[key_.value()] = Array{};
            Node* new_node = &dict[key_.value()];
            nodes_stack_.push_back(new_node);
            key_.reset();
        }

        return ArrayItemContext(*this);
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("");
        }

        if (!nodes_stack_.back()->IsDict()) {
            throw std::logic_error("");
        }

        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("");
        }

        if (!nodes_stack_.back()->IsArray()) {
            throw std::logic_error("");
        }

        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("");
        }

        if (root_.IsNull()) {
            throw std::logic_error("");
        }

        return root_;
    }


    Builder::KeyContext Builder::DictItemContext::Key(std::string key) {
        return KeyContext(builder_.Key(key));
    }

    Builder& Builder::DictItemContext::EndDict() {
        return builder_.EndDict();
    }


    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        return ArrayItemContext(builder_.Value(std::move(value)));
    }

    Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }


    Builder::DictItemContext Builder::KeyContext::Value(Node::Value value) {
        return DictItemContext(builder_.Value(std::move(value)));
    }

    Builder::DictItemContext Builder::KeyContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayItemContext Builder::KeyContext::StartArray() {
        return builder_.StartArray();
    }

}