#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
    /**
     * @brief Класс Formula представляет формулу и реализует интерфейс FormulaInterface.
     */
    class Formula : public FormulaInterface {
        public:
            /**
             * @brief Конструктор класса Formula.
             * 
             * @param expression Строка с выражением формулы.
             * @throws FormulaException Если выражение формулы имеет синтаксические ошибки.
             */
            explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression)) {}
            catch (...) {
                throw FormulaException("formula is syntactically incorrect");
            }

            /**
             * @brief Вычисляет значение формулы с использованием данных из переданного листа.
             * 
             * @param sheet Ссылка на интерфейс таблицы, в которой содержатся ячейки.
             * @return Результат вычисления формулы (значение или ошибка).
             */
            Value Evaluate(const SheetInterface& sheet) const {
                try {
                    std::function<double(Position)> params = [&sheet](const Position pos) -> double {
                        if (!pos.IsValid()) {
                            throw FormulaError(FormulaError::Category::Ref);
                        }

                        const auto* cell = sheet.GetCell(pos);
                        if (!cell) {
                            return 0.0;
                        }

                        const auto& cellValue = cell->GetValue();
                        if (std::holds_alternative<double>(cellValue)) {
                            return std::get<double>(cellValue);
                        }

                        if (std::holds_alternative<std::string>(cellValue)) {
                            const auto& strValue = std::get<std::string>(cellValue);
                            if (strValue.empty()) {
                                return 0.0;
                            }

                            std::istringstream input(strValue);
                            double digit = 0.0;
                            if (input >> digit && input.eof()) {
                                return digit;
                            }

                            throw FormulaError(FormulaError::Category::Value);
                        }

                        throw FormulaError(std::get<FormulaError>(cellValue));
                    };

                    return ast_.Execute(params);
                } catch (const FormulaError& evaluate_error) {
                    return evaluate_error;
                }
            }

            /**
             * @brief Возвращает строковое представление выражения формулы.
             * 
             * @return Строковое представление выражения формулы.
             */
            std::string GetExpression() const override {
                std::ostringstream out;
                ast_.PrintFormula(out);
                return out.str();
            }

            /**
             * @brief Возвращает список позиций ячеек, на которые ссылается формула.
             * 
             * @return Список позиций ячеек, на которые ссылается формула.
             */
            std::vector<Position> GetReferencedCells() const override {
                std::set<Position> cells;
                for (const auto& cell : ast_.GetCells()) {

                    if (cell.IsValid()) {
                        cells.insert(cell);
                    } else {
                        continue;
                    }
                }
                return std::vector<Position>(cells.cbegin(), cells.cend());
            }

        private:
            FormulaAST ast_; /**< Объект для работы с AST (абстрактным синтаксическим деревом) формулы. */
    };

}//end namespace

/**
 * @brief Функция для разбора формулы и создания объекта, реализующего интерфейс FormulaInterface.
 * 
 * @param expression Строка с выражением формулы.
 * @return Указатель на объект FormulaInterface, представляющий формулу.
 */
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
