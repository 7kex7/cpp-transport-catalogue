#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    obj_pointers_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    RenderContext context(out, 0, 2);
    for (const auto& ptr : obj_pointers_) {
        ptr->Render(context);
    }
    out << "</svg>";
}


// ---------- Circle ------------------

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    this->RenderAttrs(out);
    out << "/>"sv;
}

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}


// ---------- Polyline ------------------

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (const Point& point : points_) {
        if (!is_first) {
            out << " "sv;
        }
        is_first = false;
        out << point.x << ","sv << point.y;
    }
    out << "\""sv;
    this->RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

// ---------- Text ------------------

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    out << " x=\""sv << pos_.x << "\""sv << " y=\""sv << pos_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\""sv << " dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << size_ << "\""sv;
    if (font_family_ != "") {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (font_weight_ != "") {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    this->RenderAttrs(out);
    out << ">"sv;
    out << data_;
    out << "</text>"sv;
}

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string text) {
    std::string data;
    for (const char& c : text) {
        if (c == '"') {
            data += "&quot;";
        } else if (c == '\'') {
            data += "&apos;";
        } else if (c == '<') {
            data += "&lt;";
        } else if (c == '>') {
            data += "&gt;";
        } else if (c == '&') {
            data += "&amp;";
        } else {
            data += c;
        }
    }
    data_ = std::move(data);
    return *this;
}

}  // namespace svg