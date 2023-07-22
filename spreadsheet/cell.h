#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <stack>
#include <set>
#include <optional>

class Sheet;

/**
 * @brief Перечисление, представляющее состояния ячейки при обходе графа зависимостей.
 */
enum class CellState { NotVisited, Visiting, Visited };

/**
 * @brief Класс, представляющий ячейку таблицы.
 */
class Cell : public CellInterface {
    public:
        /**
         * @brief Конструктор ячейки.
         * 
         * @param sheet Ссылка на объект таблицы, к которой принадлежит ячейка.
         */
        Cell(Sheet& sheet);

        /**
         * @brief Деструктор ячейки.
         */
        ~Cell();

        /**
         * @brief Устанавливает значение ячейки и обрабатывает формулы.
         * 
         * @param text Текст, который будет установлен в ячейку (может быть формулой).
         * @param pos Позиция ячейки для которой устанавливается значение.
         * @param sheet Указатель на объект таблицы, в которую входит ячейка.
         */
        void Set(std::string text, Position pos, Sheet* sheet);

        /**
         * @brief Очищает содержимое ячейки, устанавливая пустую строку.
         */
        void Clear();

        /**
         * @brief Возвращает значение ячейки.
         * 
         * @return Значение ячейки.
         */
        Value GetValue() const override;

        /**
         * @brief Возвращает текстовое представление значения ячейки.
         * 
         * @return Текстовое представление значения ячейки.
         */
        std::string GetText() const override;

        /**
         * @brief Проверяет наличие циклических зависимостей для заданной ячейки.
         * 
         * @param referencedCells Список позиций ячеек, на которые ссылается формула текущей ячейки.
         * @return true, если найдены циклические зависимости, в противном случае - false.
         */
        bool CheckCircularDependency(const std::vector<Position>& referencedCells) const;

        /**
         * @brief Возвращает список позиций ячеек, на которые ссылается формула текущей ячейки.
         * 
         * @return Список позиций ячеек, на которые ссылается формула текущей ячейки.
         */
        std::vector<Position> GetReferencedCells() const override;

        /**
         * @brief Проверяет, является ли ячейка зависимой (используется формула).
         * 
         * @return true, если ячейка зависимая, в противном случае - false.
         */
        bool IsReferenced() const;

        /**
         * @brief Сбрасывает кеш ячейки и всех зависимых от нее ячеек.
         * 
         * @param force Принудительное обновление кеша (по умолчанию - false).
         */
        void ResetCache(bool force = false);

    private:
        class Impl;
        /**
         * @brief Вспомогательная функция для проверки наличия циклических зависимостей в графе.
         * 
         * @param cell Указатель на ячейку, для которой проверяется наличие циклических зависимостей.
         * @param visitedPos Множество посещенных ячеек для предотвращения зацикливания.
         * @param pos_const Константная ссылка на позицию исходной ячейки для проверки.
         * @return true, если найдены циклические зависимости, в противном случае - false.
         */
        bool HasCircularDependency(Cell* cell, std::unordered_set<Cell*>& visitedPos, const Position pos_const);

        /**
         * @brief Проверяет наличие циклических зависимостей для новой формулы ячейки.
         * 
         * @param new_impl Константная ссылка на объект новой реализации ячейки (новая формула).
         * @param pos Позиция ячейки для которой устанавливается новая формула.
         * @return true, если найдены циклические зависимости, в противном случае - false.
         */
        bool CheckCircularDependencies(const Impl& new_impl, Position pos);

        /**
         * @brief Абстрактный базовый класс для реализации различных типов ячеек.
         */
        class Impl {
            public:
                /**
                 * @brief Возвращает значение ячейки.
                 * 
                 * @return Значение ячейки.
                 */
                virtual Value GetValue() const = 0;

                /**
                 * @brief Возвращает текстовое представление значения ячейки.
                 * 
                 * @return Текстовое представление значения ячейки.
                 */
                virtual std::string GetText() const = 0;

                /**
                 * @brief Возвращает список позиций ячеек, на которые ссылается формула текущей ячейки.
                 * 
                 * @return Список позиций ячеек, на которые ссылается формула текущей ячейки.
                 */
                virtual std::vector<Position> GetReferencedCells() const;

                /**
                 * @brief Проверяет, есть ли в кеше значение, вычисленное с помощью формулы.
                 * 
                 * @return true, если значение есть в кеше, в противном случае - false.
                 */
                virtual bool HasCache();

                /**
                 * @brief Сбрасывает кеш ячейки, чтобы пересчитать значение при следующем запросе.
                 */
                virtual void ResetCache();

                /**
                 * @brief Виртуальный деструктор для полиморфного использования.
                 */
                virtual ~Impl() = default;
        };

        /**
         * @brief Конкретная реализация пустой ячейки (без значения).
         */
        class EmptyImpl : public Impl {
            public:
                Value GetValue() const override;
                std::string GetText() const override;
        };

        /**
         * @brief Конкретная реализация ячейки с текстовым значением.
         */
        class TextImpl : public Impl {
            public:
                /**
                 * @brief Конструктор текстовой реализации ячейки.
                 * 
                 * @param text Текстовое значение ячейки.
                 */
                explicit TextImpl(std::string text);

                Value GetValue() const override;
                std::string GetText() const override;

            private:
                std::string text_;
        };

        /**
         * @brief Конкретная реализация ячейки с формулой.
         */
        class FormulaImpl : public Impl {
            public:
                /**
                 * @brief Конструктор реализации ячейки с формулой.
                 * 
                 * @param text Текстовое значение формулы ячейки.
                 * @param sheet Ссылка на интерфейс таблицы, в которой содержится ячейка.
                 */
                explicit FormulaImpl(std::string text, SheetInterface& sheet);

                Value GetValue() const override;
                std::string GetText() const override;
                std::vector<Position> GetReferencedCells() const override;

                bool HasCache() override;
                void ResetCache() override;

            private:
                mutable std::optional<FormulaInterface::Value> cache_;
                std::unique_ptr<FormulaInterface> formula_ptr_;
                SheetInterface& sheet_;
        };

        std::unique_ptr<Impl> impl_; /**< Указатель на реализацию ячейки (пустой, текстовый, или формула). */
        Sheet& sheet_; /**< Ссылка на объект таблицы, к которой принадлежит ячейка. */
        std::unordered_set<Cell*> calculated_cells_; /**< Множество указателей на ячейки, которые используют текущую ячейку в формуле. */
        std::unordered_set<Cell*> using_cells_; /**< Множество указателей на ячейки, на которые ссылается текущая ячейка через формулу. */
        Position pos_; /**< Позиция ячейки в таблице. */
};
