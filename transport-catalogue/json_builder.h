#pragma once
#include "json.h"
#include <optional>

namespace json {

	class Builder;
	class DictItemContext;
	class ArrayItemContext;
	class KeyContext;

	class DictItemContext {
	public:
		DictItemContext(Builder& builder)
			: builder_(builder)
		{
		}

		KeyContext Key(std::string key);

		Builder& EndDict();

	private:
		Builder& builder_;
	};

	class ArrayItemContext {
	public:
		ArrayItemContext(Builder& builder)
			: builder_(builder)
		{
		}

		ArrayItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndArray();

	private:
		Builder& builder_;
	};

	class KeyContext {
	public:
		KeyContext(Builder& builder)
			: builder_(builder)
		{
		}

		DictItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();

	private:
		Builder& builder_;
	};

	class Builder {
	public:
		Builder()
			: root_(nullptr)
		{
		}

		Builder& Key(std::string key) {
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

		Builder& Value(Node::Value value) {

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

		DictItemContext StartDict() {
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

		ArrayItemContext StartArray() {
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

		Builder& EndDict() {
			if (nodes_stack_.empty()) {
				throw std::logic_error("");
			}

			if (!nodes_stack_.back()->IsDict()) {
				throw std::logic_error("");
			}

			nodes_stack_.pop_back();
			return *this;
		}

		Builder& EndArray() {
			if (nodes_stack_.empty()) {
				throw std::logic_error("");
			}

			if (!nodes_stack_.back()->IsArray()) {
				throw std::logic_error("");
			}

			nodes_stack_.pop_back();
			return *this;
		}

		Node Build() {
			if (!nodes_stack_.empty()) {
				throw std::logic_error("");
			}

			if (root_.IsNull()) {
				throw std::logic_error("");
			}

			return root_;
		}

	private:
		Node root_;
		std::vector<Node*> nodes_stack_{};
		std::optional<std::string> key_;
	};

}