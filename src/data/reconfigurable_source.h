#pragma once

#include <bitset>

#include "xgboost/data.h"

namespace xgboost {
namespace data {

struct ReconfigurableBatch {
  using ConfigState = std::bitset<256>;

  ReconfigurableBatch() = default;
  ReconfigurableBatch(SparsePage rows)
      : rows_(new SparsePage(std::move(rows))) {}

  ConfigState config_state_{0};

  MetaInfo info_;

  std::unique_ptr<SparsePage> rows_;  // csr data
  std::unique_ptr<SparsePage> cols_;  // csc data
};

struct ReconfigurableSource;
using ReconfigurableSourcePtr = std::shared_ptr<ReconfigurableSource>;

using ColCreatorFn = std::function<void(size_t const, bst_uint*, bst_float*)>;

struct ReconfigurableSource : public DataSource {
  ReconfigurableSource() = default;
  ReconfigurableSource(std::vector<ReconfigurableBatch> batches)
      : batches_{std::move(batches)} {}
  virtual ~ReconfigurableSource() override = default;

  static ReconfigurableSourcePtr Create(
      DMatrix* mat, std::vector<std::vector<size_t>> const& indices);

  static ReconfigurableSourcePtr Create(
      size_t nrow, std::vector<size_t> const& col_widths,
      std::vector<ColCreatorFn> const& col_creators, double const* labels,
      double const* weights, std::vector<std::vector<size_t>> const& indices);

  void LazyInitializeColumns();
  void MaybeReconfigure(ReconfigurableBatch::ConfigState);

  bool Next() override;
  void BeforeFirst() override;
  const SparsePage& Value() const override;

  std::vector<ReconfigurableBatch> batches_;
};

}  // namespace data
}  // namespace xgboost
