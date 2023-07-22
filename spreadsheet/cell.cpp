#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <set>
#include <stack>

/**
 * @brief Конструктор ячейки.
 * 
 * @param sheet Ссылка на объект таблицы, к которой принадлежит ячейка.
 */
Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

/**
 * @brief Деструктор ячейки.
 */
Cell::~Cell() = default;

/**
 * @brief Устанавливает значение ячейки и обрабатывает формулы.
 * 
 * @param text Текст, который будет установлен в ячейку (может быть формулой).
 * @param pos Позиция ячейки для которой устанавливается значение.
 * @param sheet Указатель на объект таблицы, в которую входит ячейка.
 */
void Cell::Set(std::string text, Position pos, Sheet* sheet) {
    std::unique_ptr<Impl> impl;
    std::forward_list<Position> set_formula_cells;

    if (text.empty()) {
        impl = std::make_unique<EmptyImpl>();
    } else if (text.size() >= 2 && text[0] == FORMULA_SIGN) {
        impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
        std::vector<Position> pos_cell_in_formula = impl->GetReferencedCells();
        for (Position pos_cell_in_formula : pos_cell_in_formula ) {
            if (pos_cell_in_formula.IsValid() && !sheet->GetCell(pos_cell_in_formula)) {
                sheet->SetCell(pos_cell_in_formula, "");
            }
        }

    } else {
        impl = std::make_unique<TextImpl>(std::move(text));
    }

    if (CheckCircularDependencies(*impl, pos)) {
        throw CircularDependencyException("circular dependency detected");
    }

    // Обновление зависимостей
    auto second_st = this->pos_.ToString();

    impl_ = std::move(impl);

    for (Cell* used : using_cells_) {
        auto third_st = used->pos_.ToString();
        used->calculated_cells_.erase(this);
    }

    using_cells_.clear();

    auto iterim_st = impl_->GetText();
    for (const auto& pos : impl_->GetReferencedCells()) {
        auto four_st = pos_.ToString();
        Cell* used = sheet_.GetConcreteCell(pos);
        if (!used ) {
            sheet_.SetCell(pos, "");
            used  = sheet_.GetConcreteCell(pos);
        }
        auto five_st = used->pos_.ToString();
        using_cells_.insert(used);
        used->calculated_cells_.insert(this);
    }

    // Инвалидация кэша
    ResetCache(true);
}

/**
 * @brief Проверяет наличие циклических зависимостей для заданной ячейки.
 * 
 * @param cell Указатель на ячейку, для которой проверяется наличие циклических зависимостей.
 * @param visitedPos Множество посещенных ячеек для предотвращения зацикливания.
 * @param pos_const Константная ссылка на позицию исходной ячейки для проверки.
 * @return true, если найдены циклические зависимости, в противном случае - false.
 */
bool Cell::HasCircularDependency(Cell* cell, std::unordered_set<Cell*>& visitedPos, const Position pos_const) {
    for (auto dependentPos : cell->GetReferencedCells()) {
        Cell* ref_cell = sheet_.GetConcreteCell(dependentPos);
        if (pos_const == dependentPos) {
            return true;
        }
        if (visitedPos.find(ref_cell) == visitedPos.end()) {
            visitedPos.insert(ref_cell);
            if (HasCircularDependency(ref_cell, visitedPos, pos_const))
                return true;
        }
    }
    return false;
}

/**
 * @brief Проверяет наличие циклических зависимостей для новой формулы ячейки.
 * 
 * @param new_impl Константная ссылка на объект новой реализации ячейки (новая формула).
 * @param pos Позиция ячейки для которой устанавливается новая формула.
 * @return true, если найдены циклические зависимости, в противном случае - false.
 */
bool Cell::CheckCircularDependencies(const Impl& new_impl, Position pos) {
    const Position pos_const = pos;
    const auto& cells = new_impl.GetReferencedCells();
    std::unordered_set<Cell*> visitedPos;
    for (const auto& position : cells) {
        if (position == pos) {
            return true; /*CircularDependencyException("circular dependency detected");*/
        }
        Cell* ref_cell = sheet_.GetConcreteCell(position);
        visitedPos.insert(ref_cell);
        if (HasCircularDependency(ref_cell, visitedPos, pos_const)){
            return true;
        }
    }
    return false;
}

/**
 * @brief Очищает содержимое ячейки, устанавливая пустую строку.
 */
void Cell::Clear() {
    Set("", pos_, &sheet_);
}

/**
 * @brief Возвращает значение ячейки.
 * 
 * @return Значение ячейки.
 */
Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

/**
 * @brief Возвращает текстовое представление значения ячейки.
 * 
 * @return Текстовое представление значения ячейки.
 */
std::string Cell::GetText() const {
    return impl_->GetText();
}

/**
 * @brief Возвращает список позиций ячеек, на которые ссылается формула текущей ячейки.
 * 
 * @return Список позиций ячеек, на которые ссылается формула текущей ячейки.
 */
std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

/**
 * @brief Проверяет, является ли ячейка зависимой (используется формула).
 * 
 * @return true, если ячейка зависимая, в противном случае - false.
 */
bool Cell::IsReferenced() const {
    return !calculated_cells_.empty();
}

/**
 * @brief Сбрасывает кеш ячейки и всех зависимых от нее ячеек.
 * 
 * @param force Принудительное обновление кеша (по умолчанию - false).
 */
void Cell::ResetCache(bool force /*= false*/) {
    if (impl_->HasCache() || force) {
        impl_->ResetCache();
        for (Cell* dependent : calculated_cells_) {
            dependent->ResetCache();
        }
    }
}

/**
 * @brief Возвращает пустой список позиций ячеек, так как данная реализация не содержит формулу.
 * 
 * @return Пустой список позиций ячеек.
 */
std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

/**
 * @brief Возвращает true, так как данная реализация не содержит кеш.
 * 
 * @return true.
 */
bool Cell::Impl::HasCache() {
    return true;
}

/**
 * @brief Сбрасывает кеш ячейки, так как данная реализация не содержит кеш.
 */
void Cell::Impl::ResetCache() {
    // Do nothing, as this implementation does not have cache.
}

/**
 * @brief Возвращает пустое значение для пустой реализации ячейки.
 * 
 * @return Пустое значение.
 */
Cell::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

/**
 * @brief Возвращает пустую строку для пустой реализации ячейки.
 * 
 * @return Пустая строка.
 */
std::string Cell::EmptyImpl::GetText() const {
    return "";
}

/**
 * @brief Конструктор текстовой реализации ячейки.
 * 
 * @param text Текстовое значение ячейки.
 */
Cell::TextImpl::TextImpl(std::string text) : text_(std::move(text)) {}

/**
 * @brief Возвращает значение ячейки.
 * 
 * @return Значение ячейки (пустую строку или текстовое значение).
 */
Cell::Value Cell::TextImpl::GetValue() const {
    if (text_.empty()) {
        throw FormulaException("it is empty impl, not text");
    } else if (text_.at(0) == ESCAPE_SIGN) {
        return text_.substr(1);
    } else {
        return text_;
    }
}

/**
 * @brief Возвращает текстовое представление значения ячейки.
 * 
 * @return Текстовое представление значения ячейки.
 */
std::string Cell::TextImpl::GetText() const {
    return text_;
}

/**
 * @brief Конструктор реализации ячейки с формулой.
 * 
 * @param text Текстовое значение формулы ячейки.
 * @param sheet Ссылка на интерфейс таблицы, в которой содержится ячейка.
 */
Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet) : formula_ptr_(ParseFormula(text.substr(1))),
                                                                          sheet_(sheet) {}

/**
 * @brief Возвращает значение ячейки, вычисленное с помощью формулы.
 * 
 * @return Значение ячейки, вычисленное с помощью формулы.
 */
Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_) {
        cache_ = formula_ptr_->Evaluate(sheet_);
    }
    return std::visit([](auto& helper){return Value(helper);}, *cache_);
}

/**
 * @brief Возвращает текстовое представление формулы ячейки, включая символ "FORMULA_SIGN".
 * 
 * @return Текстовое представление формулы ячейки.
 */
std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_ptr_->GetExpression();
}

/**
 * @brief Возвращает список позиций ячеек, на которые ссылается формула текущей ячейки.
 * 
 * @return Список позиций ячеек, на которые ссылается формула текущей ячейки.
 */
std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_ptr_->GetReferencedCells();
}

/**
 * @brief Возвращает true, если в кеше есть значение, вычисленное с помощью формулы, в противном случае - false.
 * 
 * @return true, если в кеше есть значение, в противном случае - false.
 */
bool Cell::FormulaImpl::HasCache() {
    return cache_.has_value();
}

/**
 * @brief Сбрасывает кеш ячейки, чтобы пересчитать значение при следующем запросе.
 */
void Cell::FormulaImpl::ResetCache() {
    cache_.reset();
}
