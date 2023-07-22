#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

/**
 * @brief Деструктор класса Sheet.
 */
Sheet::~Sheet() {}

/**
 * @brief Устанавливает значение в ячейку с заданной позицией.
 * 
 * Метод устанавливает значение в ячейку с указанной позицией. Если ячейка еще не существует, она создается.
 * При установке значения, метод также обновляет зависимости других ячеек, которые ссылаются на данную ячейку.
 * 
 * @param pos Позиция ячейки, в которую необходимо установить значение.
 * @param text Значение для установки в ячейку.
 */
void Sheet::SetCell(Position pos, std::string text) {

    if (pos.IsValid()) {
        auto sheet_pos = pos.ToString();

        cells_.resize(std::max(pos.row + 1, int(std::size(cells_))));
        cells_[pos.row].resize(std::max(pos.col + 1, int(std::size(cells_[pos.row]))));

        if (!cells_[pos.row][pos.col]) {
            cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
        }

        cells_[pos.row][pos.col]->Set(std::move(text), pos, this);

    } else {
        throw InvalidPositionException("invalid cell position. setsell");
    }
}

/**
 * @brief Получает указатель на интерфейс ячейки с заданной позицией.
 * 
 * Метод получает указатель на интерфейс ячейки с заданной позицией. Если ячейка с такой позицией не существует,
 * метод возвращает nullptr.
 * 
 * @param pos Позиция ячейки, для которой необходимо получить указатель.
 * @return Указатель на интерфейс ячейки или nullptr, если ячейка не существует.
 */
CellInterface* Sheet::GetCell(Position pos) {

    if (pos.IsValid()) {

        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {
            return cells_[pos.row][pos.col].get();
        } else {
            return nullptr;
        }

    } else {
        throw InvalidPositionException("invalid cell position. getcell");
    }
}

/**
 * @brief Получает указатель на константный интерфейс ячейки с заданной позицией.
 * 
 * Метод получает указатель на константный интерфейс ячейки с заданной позицией. Если ячейка с такой позицией
 * не существует или не содержит текстового значения, метод возвращает nullptr.
 * 
 * @param pos Позиция ячейки, для которой необходимо получить указатель.
 * @return Константный указатель на интерфейс ячейки или nullptr, если ячейка не существует или пустая.
 */
const CellInterface* Sheet::GetCell(Position pos) const {

    if (pos.IsValid()) {

        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {

            if (cells_[pos.row][pos.col].get() == nullptr) {
                return nullptr;
            } else if (cells_[pos.row][pos.col].get()->GetText().empty()) {
                return nullptr;
            } else {
                return cells_[pos.row][pos.col].get();
            }

        } else {
            return nullptr;
        }

    } else {
        throw InvalidPositionException("invalid cell position. getcell");
    }
}

/**
 * @brief Получает указатель на объект типа Cell с заданной позицией.
 * 
 * Метод получает указатель на объект типа Cell с заданной позицией. Если ячейка с такой позицией не существует,
 * метод возвращает nullptr.
 * 
 * @param pos Позиция ячейки, для которой необходимо получить указатель.
 * @return Указатель на объект типа Cell или nullptr, если ячейка не существует.
 */
Cell* Sheet::GetConcreteCell(Position pos) {

    if (pos.IsValid()) {

        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {
            return cells_[pos.row][pos.col].get();
        } else {
            return nullptr;
        }

    } else {
        throw InvalidPositionException("invalid cell position. get_cell");
    }
}

/**
 * @brief Получает константный указатель на объект типа Cell с заданной позицией.
 * 
 * Метод получает константный указатель на объект типа Cell с заданной позицией. Если ячейка с такой позицией
 * не существует, метод возвращает nullptr.
 * 
 * @param pos Позиция ячейки, для которой необходимо получить указатель.
 * @return Константный указатель на объект типа Cell или nullptr, если ячейка не существует.
 */
const Cell* Sheet::GetConcreteCell(Position pos) const {
    const Cell* const_result = GetConcreteCell(pos);
    return const_result;
}

/**
 * @brief Очищает содержимое ячейки с заданной позицией.
 * 
 * Метод очищает содержимое ячейки с заданной позицией, если такая ячейка существует.
 * После очистки, если ячейка больше не используется другими ячейками, она удаляется.
 * 
 * @param pos Позиция ячейки, которую необходимо очистить.
 */
void Sheet::ClearCell(Position pos) {

    if (pos.IsValid()) {

        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {

            if (cells_[pos.row][pos.col]) {
                cells_[pos.row][pos.col]->Clear();

                if (!cells_[pos.row][pos.col]->IsReferenced()) {
                    cells_[pos.row][pos.col].reset();
                }
            }
        }

    } else {
        throw InvalidPositionException("invalid cell position. clearcell");
    }
}

/**
 * @brief Возвращает размер печатаемой области таблицы.
 * 
 * Метод возвращает размер печатаемой области таблицы, то есть количество строк и столбцов,
 * в которых содержатся не пустые значения. Это используется для форматирования вывода таблицы.
 * 
 * @return Размер печатаемой области таблицы в виде объекта Size.
 */
Size Sheet::GetPrintableSize() const {

    Size size;

    for (int row = 0; row < int(std::size(cells_)); ++row) {

        for (int col = (int(std::size(cells_[row])) - 1); col >= 0; --col) {

            if (cells_[row][col]) {

                if (cells_[row][col]->GetText().empty()) {
                    continue;
                } else {
                    size.rows = std::max(size.rows, row + 1);
                    size.cols = std::max(size.cols, col + 1);
                    break;
                }
            }
        }
    }

    return size;
}

/**
 * @brief Выводит значения ячеек таблицы в поток вывода.
 * 
 * Метод выводит значения ячеек таблицы в поток вывода. Каждая строка таблицы выводится в отдельной строке потока,
 * значения ячеек разделены табуляцией.
 * 
 * @param output Поток вывода, в который будут выведены значения ячеек.
 */
void Sheet::PrintValues(std::ostream& output) const {

    for (int row = 0; row < GetPrintableSize().rows; ++row) {

        for (int col = 0; col < GetPrintableSize().cols; ++col) {

            if (col > 0) {
                output << '\t';
            }

            if (col < int(std::size(cells_[row]))) {

                if (cells_[row][col]) {
                    std::visit([&output](const auto& value) {output << value;}, cells_[row][col]->GetValue());
                }
            }
        }

        output << '\n';
    }
}

/**
 * @brief Выводит текстовые значения ячеек таблицы в поток вывода.
 * 
 * Метод выводит текстовые значения ячеек таблицы в поток вывода. Каждая строка таблицы выводится в отдельной строке потока,
 * значения ячеек разделены табуляцией.
 * 
 * @param output Поток вывода, в который будут выведены текстовые значения ячеек.
 */
void Sheet::PrintTexts(std::ostream& output) const {

    for (int row = 0; row < GetPrintableSize().rows; ++row) {

        for (int col = 0; col < GetPrintableSize().cols; ++col) {

            if (col) {
                output << '\t';
            }

            if (col < int(std::size(cells_[row]))) {

                if (cells_[row][col]) {
                    output << cells_[row][col]->GetText();
                }
            }
        }

        output << '\n';
    }
}

/**
 * @brief Создает новый объект типа SheetInterface.
 * 
 * Метод создает и возвращает новый объект типа SheetInterface.
 * 
 * @return Указатель на новый объект типа SheetInterface.
 */
std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
