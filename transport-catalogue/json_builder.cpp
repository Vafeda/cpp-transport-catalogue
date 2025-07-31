#include "json_builder.h"

namespace json {
	KeyContext DictItemContext::Key(std::string key) {
		return KeyContext(builder_.Key(key));
	}

	Builder& DictItemContext::EndDict() {
		return builder_.EndDict();
	}


	ArrayItemContext ArrayItemContext::Value(Node::Value value) {
		return ArrayItemContext(builder_.Value(std::move(value)));
	}

	DictItemContext ArrayItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext ArrayItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}



	DictItemContext KeyContext::Value(Node::Value value) {
		builder_.Value(std::move(value));
		return DictItemContext(builder_);
	}

	DictItemContext KeyContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext KeyContext::StartArray() {
		return builder_.StartArray();
	}
}