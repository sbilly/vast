#ifndef VAST_EXPRESSION_H
#define VAST_EXPRESSION_H

#include "vast/event.h"
#include "vast/offset.h"
#include "vast/operator.h"
#include "vast/schema.h"
#include "vast/util/operators.h"
#include "vast/util/visitor.h"

namespace vast {
namespace expr {

// Forward declarations
struct constant;
struct timestamp_extractor;
struct name_extractor;
struct id_extractor;
struct offset_extractor;
struct type_extractor;
struct relation;
struct conjunction;
struct disjunction;

using const_visitor = util::const_visitor<
  constant,
  timestamp_extractor,
  name_extractor,
  id_extractor,
  offset_extractor,
  type_extractor,
  relation,
  conjunction,
  disjunction
>;

/// The base class for nodes in the expression tree.
struct node : public util::visitable_with<const_visitor>,
              util::totally_ordered<node>
{
  virtual ~node() = default;
  virtual node* clone() const = 0;
  virtual bool equals(node const& other) const = 0;
  virtual bool is_less_than(node const& other) const = 0;
  virtual void serialize(serializer& sink) const = 0;
  virtual void deserialize(deserializer& source) = 0;
  friend bool operator==(node const& x, node const& y);
};

/// A constant value.
struct constant : public util::visitable<node, constant, const_visitor>
{
  constant() = default;
  constant(value v);
  virtual constant* clone() const override;
  virtual bool equals(node const& other) const override;
  virtual bool is_less_than(node const& other) const override;
  virtual void serialize(serializer& sink) const override;
  virtual void deserialize(deserializer& source) override;

  value val;
};

/// The base class for extractor nodes.
struct extractor : public util::abstract_visitable<node, const_visitor>
{
  extractor* clone() const = 0;
  virtual bool equals(node const& other) const override;
  virtual bool is_less_than(node const& other) const override;
};

/// Extracts the event timestamp.
struct timestamp_extractor
  : public util::visitable<extractor, timestamp_extractor, const_visitor>
{
  timestamp_extractor* clone() const override;
  virtual void serialize(serializer& sink) const override;
  virtual void deserialize(deserializer& source) override;
};

/// Extracts the event name.
struct name_extractor
  : public util::visitable<extractor, name_extractor, const_visitor>
{
  name_extractor* clone() const override;
  virtual void serialize(serializer& sink) const override;
  virtual void deserialize(deserializer& source) override;
};

/// Extracts the event ID.
struct id_extractor
  : public util::visitable<extractor, id_extractor, const_visitor>
{
  id_extractor* clone() const override;
  virtual void serialize(serializer& sink) const override;
  virtual void deserialize(deserializer& source) override;
};

/// Extracts an argument at a given offset.
struct offset_extractor
  : public util::visitable<extractor, offset_extractor, const_visitor>
{
  offset_extractor() = default;
  offset_extractor(offset o);

  virtual offset_extractor* clone() const override;
  virtual bool equals(node const& other) const override;
  virtual bool is_less_than(node const& other) const override;
  virtual void serialize(serializer& sink) const override;
  virtual void deserialize(deserializer& source) override;

  offset off;
};

/// Extracts arguments of a given type.
struct type_extractor
  : public util::visitable<extractor, type_extractor, const_visitor>
{
  type_extractor() = default;
  type_extractor(value_type t);

  virtual type_extractor* clone() const override;
  virtual bool equals(node const& other) const override;
  virtual bool is_less_than(node const& other) const override;
  virtual void serialize(serializer& sink) const override;
  virtual void deserialize(deserializer& source) override;

  value_type type;
};

/// An n-ary operator.
struct n_ary_operator : public util::abstract_visitable<node, const_visitor>
{
  n_ary_operator() = default;
  n_ary_operator(n_ary_operator const& other);
  virtual n_ary_operator* clone() const = 0;
  virtual bool equals(node const& other) const override;
  virtual bool is_less_than(node const& other) const override;
  virtual void serialize(serializer& sink) const override;
  virtual void deserialize(deserializer& source) override;
  void add(std::unique_ptr<node> n);

  std::vector<std::unique_ptr<node>> operands;
};

/// A relational operator.
struct relation
  : public util::visitable<n_ary_operator, relation, const_visitor>
{
  using binary_predicate = std::function<bool(value const&, value const&)>;
  static binary_predicate make_predicate(relational_operator op);

  relation() = default;
  relation(relational_operator op);
  virtual relation* clone() const override;
  virtual bool equals(node const& other) const override;
  virtual bool is_less_than(node const& other) const override;
  virtual void serialize(serializer& sink) const override;
  virtual void deserialize(deserializer& source) override;

  binary_predicate predicate;
  relational_operator op;
};

/// A conjunction.
struct conjunction
  : public util::visitable<n_ary_operator, conjunction, const_visitor>
{
  virtual conjunction* clone() const override;
  virtual bool equals(node const& other) const override;
  virtual bool is_less_than(node const& other) const override;
  virtual void serialize(serializer& sink) const override;
  virtual void deserialize(deserializer& source) override;
};

/// A disjunction.
struct disjunction
  : public util::visitable<n_ary_operator, disjunction, const_visitor>
{
  virtual disjunction* clone() const override;
  virtual bool equals(node const& other) const override;
  virtual bool is_less_than(node const& other) const override;
  virtual void serialize(serializer& sink) const override;
  virtual void deserialize(deserializer& source) override;
};

/// A wrapper around an expression node with value semantics.
class ast : util::totally_ordered<ast>
{
public:
  ast() = default;
  ast(std::string const& str, schema const& sch = {});
  ast(std::unique_ptr<node> n);
  ast(ast const& other);
  ast(ast&&) = default;
  ast& operator=(ast const& other);
  ast& operator=(ast&& other);
  explicit operator bool() const;

  void accept(const_visitor& v);
  void accept(const_visitor& v) const;

  node const* root() const;

private:
  std::unique_ptr<node> node_;

private:
  friend access;
  void serialize(serializer& sink) const;
  void deserialize(deserializer& source);
  bool convert(std::string& str) const;
  friend bool operator<(ast const& x, ast const& y);
  friend bool operator==(ast const& x, ast const& y);
};

/// Creates an expression tree.
/// @param str The string representing the expression.
/// @param sch The schema to use to resolve event clauses.
std::unique_ptr<node> create(std::string const& str, schema const& sch = {});

/// Evaluates an expression node for a given event.
/// @relates evaluator
value evaluate(node const& n, event const& e);
value evaluate(ast const& a, event const& e);

bool convert(node const& n, std::string& str);

} // namespace expr
} // namespace vast

#endif
