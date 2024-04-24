#include <stdexcept>
#include <utility>

#include "json_builder.h"

using namespace std::literals;

namespace json {

Builder::Builder() = default;

void Builder::AddToArray(Node& container_node, std::unique_ptr<Node>&& value_ptr) {
    Node::Value& parent_arr = container_node.GetValue();
    std::get<Array>(parent_arr).push_back(*value_ptr);
}
void Builder::AddToDict(Node& container_node, Node& value) {
    Dict& dict = std::get<Dict>(container_node.GetValue());

    if (key_stack_.empty()) {
        throw std::logic_error("Value: there is no key for given value"s);
    }

    std::string& last_added_key = key_stack_.back();
    if (dict.count(last_added_key)) {
        throw std::logic_error("Value: element with the key already exists"s);
    }
    dict.insert({last_added_key, value});
    key_stack_.pop_back();
}

Builder::ArrayInitContext Builder::StartArray() {
    if (root_.has_value() && nodes_stack_.empty()) {
        throw std::logic_error("StartArray: trying to add the second element without container (as array)"s);
    }
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsDict() && last_cmd != Commands::KEY) {
            throw std::logic_error("StartArray: trying to add the array as a key"s);
        }
    }

    nodes_stack_.emplace_back(new Node(Array{}));

    last_cmd = Commands::START_ARRAY;
    return ArrayInitContext{*this};
}

Builder& Builder::EndArray() {
    if (root_.has_value() || nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("err: array completion is not possible"s);
    }

    std::unique_ptr<Node> current_arr_ptr = std::move(nodes_stack_.back());
    nodes_stack_.pop_back();
    if (nodes_stack_.size() != 0) {
        Node& parent_node = *nodes_stack_.back();
        if (parent_node.IsArray()) {
            AddToArray(parent_node, std::move(current_arr_ptr));
        } else if (parent_node.IsDict()) {
            AddToDict(parent_node, *current_arr_ptr);
        } else {
            throw std::logic_error("EndArray: unrecognized type of parent"s);
        }
    } else {
        root_ = *current_arr_ptr;
    }

    last_cmd = Commands::END_ARRAY;
    return *this;
}


Builder::DictInitContext Builder::StartDict() {
    if (root_.has_value() && nodes_stack_.empty()) {
        throw std::logic_error("StartDict: trying to add a second element without container (as dictionary)"s);
    }
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsDict() && last_cmd != Commands::KEY) {
            throw std::logic_error("StartDict: trying to add the dict as a key"s);
        }
    }

    nodes_stack_.emplace_back(new Node(Dict{}));
    
    last_cmd = Commands::START_DICT;
    return DictInitContext{*this};
}

Builder& Builder::EndDict() {
    if (root_.has_value() || nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("EndDict: dict completion is not possible"s);
    }

    std::unique_ptr<Node> current_dict_ptr = std::move(nodes_stack_.back());
    nodes_stack_.pop_back();
    if (nodes_stack_.size() != 0) {
        Node& parent_node = *nodes_stack_.back();
        if (parent_node.IsArray()) {
            AddToArray(parent_node, std::move(current_dict_ptr));
        } else if (parent_node.IsDict()) {
            AddToDict(parent_node, *current_dict_ptr);
        } else {
            throw std::logic_error("EndDict: unrecognized type of parent"s);
        }
    } else {
        root_ = *current_dict_ptr;
    }

    last_cmd = Commands::END_DICT;
    return *this;
}


Builder::DictItemContext Builder::Key(const std::string& key) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Key: trying to add the key without container"s);
    } else if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Key: trying to create key outside dictionary"s);
    } else if (last_cmd == Commands::KEY) {
        throw std::logic_error("Key: trying to add the second key immediately"s);
    } else if (last_cmd == Commands::START_ARRAY) {
        throw std::logic_error("Key: trying to add the key in array"s);
    } else {
        key_stack_.push_back(key);
    }

    last_cmd = Commands::KEY;
    return DictItemContext{*this};
}


Builder& Builder::Value(const Node::Value& value) {
    if (nodes_stack_.empty()) {
        if (root_.has_value()) {
            throw std::logic_error("Value: trying to add the second element without container (as value)"s);
        }
        root_ = value;

    } else if (nodes_stack_.back()->IsArray()) {
        Node::Value& parent_arr = nodes_stack_.back()->GetValue();
        std::get<Array>(parent_arr).emplace_back(value);

    } else if (nodes_stack_.back()->IsDict()) {
        if (last_cmd != Commands::KEY) {
            throw std::logic_error("Value: trying to add a value without key"s);
        }

        Node& parent_dict = *nodes_stack_.back();
        Node node_value = Node(value);
        AddToDict(parent_dict, node_value);

    } else {
        throw std::logic_error("Value: unrecognized type of parent"s);
    }

    last_cmd = Commands::VALUE;
    return *this;
}


Node& Builder::Build() {
    if (nodes_stack_.empty() && !root_.has_value()) {
        throw std::logic_error("Build: construction immediately after initialization"s);
    }
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Build: unfinished container building"s);
    }
    if (last_cmd == Commands::KEY) {
        throw std::logic_error("Build: key without value"s);
    }

    return *root_;
}

} // json