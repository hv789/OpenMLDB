/*-------------------------------------------------------------------------
 * Copyright (C) 2020, 4paradigm
 * physical_op.cc
 *
 * Author: chenjing
 * Date: 2020/3/12
 *--------------------------------------------------------------------------
 **/
#include "vm/physical_op.h"
namespace fesql {
namespace vm {

const char INDENT[] = "  ";
void PhysicalOpNode::Print(std::ostream& output, const std::string& tab) const {
    output << tab << PhysicalOpTypeName(type_);
}

void PhysicalOpNode::Print() const { this->Print(std::cout, "    "); }

void PhysicalOpNode::PrintChildren(std::ostream& output,
                                   const std::string& tab) const {}
void PhysicalOpNode::UpdateProducer(int i, PhysicalOpNode* producer) {
    producers_[i] = producer;
}
void PhysicalUnaryNode::PrintChildren(std::ostream& output,
                                      const std::string& tab) const {
    if (producers_.empty() || nullptr == producers_[0]) {
        LOG(WARNING) << "empty producers";
        return;
    }
    producers_[0]->Print(output, tab + INDENT);
}
void PhysicalUnaryNode::Print(std::ostream& output,
                              const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    if (limit_cnt_ > 0) {
        output << "(limit=" << limit_cnt_ << ")";
    }
    output << "\n";
    PrintChildren(output, tab);
}
bool PhysicalUnaryNode::InitSchema() {
    if (producers_.empty() || nullptr == producers_[0]) {
        LOG(WARNING) << "InitSchema fail: producers is empty or null";
        return false;
    }
    output_schema_.CopyFrom(producers_[0]->output_schema_);
    for (auto pair : producers_[0]->GetOutputNameSchemaList()) {
        output_name_schema_list_.push_back(pair);
    }
    PrintSchema();
    return true;
}

void PhysicalBinaryNode::PrintChildren(std::ostream& output,
                                       const std::string& tab) const {
    if (2 != producers_.size() || nullptr == producers_[0] ||
        nullptr == producers_[1]) {
        LOG(WARNING) << "fail to print children";
        return;
    }
    producers_[0]->Print(output, tab + INDENT);
    output << "\n";
    producers_[1]->Print(output, tab + INDENT);
}
void PhysicalBinaryNode::Print(std::ostream& output,
                               const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "\n";
    PrintChildren(output, tab);
}
void PhysicalTableProviderNode::Print(std::ostream& output,
                                      const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(table=" << table_handler_->GetName() << ")";
}

void PhysicalRequestProviderNode::Print(std::ostream& output,
                                        const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(request=" << table_handler_->GetName() << ")";
}

void PhysicalPartitionProviderNode::Print(std::ostream& output,
                                          const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(type=" << DataProviderTypeName(provider_type_)
           << ", table=" << table_handler_->GetName()
           << ", index=" << index_name_ << ")";
}

void PhysicalGroupNode::Print(std::ostream& output,
                              const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "("
           << "group_" << group_.ToString() << ")";
    output << "\n";
    PrintChildren(output, tab);
}
PhysicalGroupNode* PhysicalGroupNode::CastFrom(PhysicalOpNode* node) {
    return dynamic_cast<PhysicalGroupNode*>(node);
}

void PhysicalProjectNode::Print(std::ostream& output,
                                const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(type=" << ProjectTypeName(project_type_);
    if (limit_cnt_ > 0) {
        output << ", limit=" << limit_cnt_;
    }
    output << ")";
    output << "\n";
    PrintChildren(output, tab);
}
bool PhysicalProjectNode::InitSchema() {
    output_name_schema_list_.push_back(std::make_pair("", &output_schema_));
    PrintSchema();
    return true;
}

PhysicalProjectNode* PhysicalProjectNode::CastFrom(PhysicalOpNode* node) {
    return dynamic_cast<PhysicalProjectNode*>(node);
}

PhysicalRowProjectNode* PhysicalRowProjectNode::CastFrom(PhysicalOpNode* node) {
    return dynamic_cast<PhysicalRowProjectNode*>(node);
}

PhysicalTableProjectNode* PhysicalTableProjectNode::CastFrom(
    PhysicalOpNode* node) {
    return dynamic_cast<PhysicalTableProjectNode*>(node);
}

PhysicalWindowAggrerationNode* PhysicalWindowAggrerationNode::CastFrom(
    PhysicalOpNode* node) {
    return dynamic_cast<PhysicalWindowAggrerationNode*>(node);
}

void PhysicalGroupAggrerationNode::Print(std::ostream& output,
                                         const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(type=" << ProjectTypeName(project_type_) << ", "
           << "group_" << group_.ToString();
    if (limit_cnt_ > 0) {
        output << ", limit=" << limit_cnt_;
    }
    output << ")";
    output << "\n";
    PrintChildren(output, tab);
}

void PhysicalWindowAggrerationNode::Print(std::ostream& output,
                                          const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(type=" << ProjectTypeName(project_type_) << ", "
           << window_.ToString();
    if (limit_cnt_ > 0) {
        output << ", limit=" << limit_cnt_;
    }
    output << ")";
    output << "\n";
    PrintChildren(output, tab);
}
bool PhysicalWindowAggrerationNode::InitSchema() {
    // TODO(chenjing): Init Schema with window Join
    return false;
}

void PhysicalJoinNode::Print(std::ostream& output,
                             const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(type=" << node::JoinTypeName(join_type_) << ", "
           << join_.ToString();
    if (limit_cnt_ > 0) {
        output << ", limit=" << limit_cnt_;
    }
    output << ")";
    output << "\n";
    PrintChildren(output, tab);
}
bool PhysicalJoinNode::InitSchema() {
    if (2 != producers_.size() || nullptr == producers_[0] ||
        nullptr == producers_[1]) {
        LOG(WARNING) << "InitSchema fail: producers size isn't 2 or left/right "
                        "producer is null";
        return false;
    }
    output_schema_.CopyFrom(producers_[0]->output_schema_);
    output_schema_.MergeFrom(producers_[1]->output_schema_);
    for (auto pair : producers_[0]->GetOutputNameSchemaList()) {
        output_name_schema_list_.push_back(pair);
    }
    for (auto right_pair : producers_[1]->GetOutputNameSchemaList()) {
        output_name_schema_list_.push_back(right_pair);
    }
    PrintSchema();
    return true;
}
PhysicalJoinNode* PhysicalJoinNode::CastFrom(PhysicalOpNode* node) {
    return dynamic_cast<PhysicalJoinNode*>(node);
}

void PhysicalSortNode::Print(std::ostream& output,
                             const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(" << sort_.ToString();
    if (limit_cnt_ > 0) {
        output << ", limit=" << limit_cnt_;
    }
    output << ")";
    output << "\n";
    PrintChildren(output, tab);
}
void PhysicalLimitNode::Print(std::ostream& output,
                              const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(limit=" << std::to_string(limit_cnt_)
           << (limit_optimized_ ? ", optimized" : "") << ")";
    output << "\n";
    PrintChildren(output, tab);
}
void PhysicalRenameNode::Print(std::ostream& output,
                               const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(name=" << name_ << ")";
    output << "\n";
    PrintChildren(output, tab);
}
void PhysicalFliterNode::Print(std::ostream& output,
                               const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(" << filter_.ToString();
    if (limit_cnt_ > 0) {
        output << ", limit=" << limit_cnt_;
    }
    output << ")";
    output << "\n";
    PrintChildren(output, tab);
}

PhysicalDataProviderNode* PhysicalDataProviderNode::CastFrom(
    PhysicalOpNode* node) {
    return dynamic_cast<PhysicalDataProviderNode*>(node);
}

const std::string& PhysicalDataProviderNode::GetName() const {
    return table_handler_->GetName();
}

bool PhysicalDataProviderNode::InitSchema() {
    if (table_handler_) {
        auto schema = table_handler_->GetSchema();
        if (schema) {
            output_schema_.CopyFrom(*schema);
            output_name_schema_list_.push_back(std::make_pair(
                table_handler_->GetName(), table_handler_->GetSchema()));
            PrintSchema();
            return true;
        } else {
            LOG(WARNING) << "InitSchema fail: table schema is null";
            return false;
        }
    } else {
        LOG(WARNING) << "InitSchema fail: table handler is null";
        return false;
    }
}
void PhysicalOpNode::PrintSchema() {
    std::stringstream ss;
    ss << PhysicalOpTypeName(type_) << " output name schema list: \n";
    for (auto pair : GetOutputNameSchemaList()) {
        ss << "pair table: " << pair.first << "\n";
        for (int32_t i = 0; i < pair.second->size(); i++) {
            if (i > 0) {
                ss << "\n";
            }
            const type::ColumnDef& column = pair.second->Get(i);
            ss << column.name() << " " << type::Type_Name(column.type());
        }
        ss << "\n";
    }
    ss << "output schema\n";
    for (int32_t i = 0; i < output_schema_.size(); i++) {
        if (i > 0) {
            ss << "\n";
        }
        const type::ColumnDef& column = output_schema_.Get(i);
        ss << column.name() << " " << type::Type_Name(column.type());
    }
    DLOG(INFO) << "\n" << ss.str();
}
bool PhysicalUnionNode::InitSchema() {
    output_schema_.CopyFrom(producers_[0]->output_schema_);
    for (auto pair : producers_[0]->GetOutputNameSchemaList()) {
        output_name_schema_list_.push_back(pair);
    }
    PrintSchema();
    return true;
}
void PhysicalUnionNode::Print(std::ostream& output,
                              const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "\n";
    PrintChildren(output, tab);
}

void PhysicalRequestUnionNode::Print(std::ostream& output,
                                     const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(";
    output << window_.ToString();
    output << ", index_" << index_key_.ToString();
    if (limit_cnt_ > 0) {
        output << ", limit=" << limit_cnt_;
    }
    output << ")";
    output << "\n";
    PrintChildren(output, tab);
}
bool PhysicalRequestUnionNode::InitSchema() {
    output_schema_.CopyFrom(producers_[0]->output_schema_);
    for (auto pair : producers_[0]->GetOutputNameSchemaList()) {
        output_name_schema_list_.push_back(pair);
    }
    PrintSchema();
    return true;
}

void PhysicalRequestJoinNode::Print(std::ostream& output,
                                    const std::string& tab) const {
    PhysicalOpNode::Print(output, tab);
    output << "(type=" << node::JoinTypeName(join_type_) << ", "
           << join_.ToString();
    if (limit_cnt_ > 0) {
        output << ", limit=" << limit_cnt_;
    }
    output << ")";
    output << "\n";
    PrintChildren(output, tab);
}

bool PhysicalRequestJoinNode::InitSchema() {
    if (2 != producers_.size() || nullptr == producers_[0] ||
        nullptr == producers_[1]) {
        LOG(WARNING) << "InitSchema fail: producers size isn't 2 or left/right "
                        "producer is null";
        return false;
    }
    output_schema_.CopyFrom(producers_[0]->output_schema_);
    output_schema_.MergeFrom(producers_[1]->output_schema_);
    for (auto pair : producers_[0]->GetOutputNameSchemaList()) {
        output_name_schema_list_.push_back(pair);
    }
    for (auto right_pair : producers_[1]->GetOutputNameSchemaList()) {
        output_name_schema_list_.push_back(right_pair);
    }
    PrintSchema();
    return true;
}

}  // namespace vm
}  // namespace fesql
