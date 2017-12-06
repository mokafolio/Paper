#ifndef PAPER_ITEM_HPP
#define PAPER_ITEM_HPP

#include <Paper/Constants.hpp>
#include <Paper/Paint.hpp>

namespace paper
{
    class Document;

    class Item;
    using ItemArray = stick::DynamicArray<Item>;

    class STICK_API Item : public brick::TypedEntity
    {
    public:

        enum class BoundsType
        {
            Fill,
            Stroke,
            Handle
        };


        Item();

        void addChild(Item _e);

        void insertAbove(Item _e);

        void insertBelow(Item _e);

        void sendToFront();

        void sendToBack();

        void remove();

        bool removeChild(Item _item);

        void removeChildren();

        void reverseChildren();

        const ItemArray & children() const;

        const stick::String & name() const;

        Item parent() const;


        void setPosition(const Vec2f & _position);

        void setPivot(const Vec2f & _pivot);

        void setVisible(bool _b);

        void setName(const stick::String & _name);


        //transformation things

        void setTransform(const Mat3f & _transform, bool _bIncludesScaling = false);

        void translateTransform(Float _x, Float _y);

        void translateTransform(const Vec2f & _translation);

        void scaleTransform(Float _scale);

        void scaleTransform(Float _scaleX, Float _scaleY);

        void scaleTransform(const Vec2f & _scale);

        void scaleTransform(const Vec2f & _scale, const Vec2f & _center);

        void rotateTransform(Float _radians);

        void rotateTransform(Float _radians, const Vec2f & _point);

        void skewTransform(const Vec2f & _angles);

        void skewTransform(const Vec2f & _angles, const Vec2f & _center);

        void transform(const Mat3f & _transform, bool _bIncludesScaling = false);


        void translate(Float _x, Float _y);

        void translate(const Vec2f & _translation);

        void scale(Float _scale);

        void scale(Float _scaleX, Float _scaleY);

        void scale(const Vec2f & _scale);

        void scale(const Vec2f & _scale, const Vec2f & _center);

        void rotate(Float _radians);

        void rotate(Float _radians, const Vec2f & _point);

        void skew(const Vec2f & _angles);

        void skew(const Vec2f & _angles, const Vec2f & _center);

        void applyTransform(const Mat3f & _transform, bool _bNotifyParent = true);


        const Mat3f & transform() const;

        const Mat3f & absoluteTransform() const;

        stick::Float32 rotation() const;

        const Vec2f & translation() const;

        const Vec2f & scaling() const;

        stick::Float32 absoluteRotation() const;

        const Vec2f & absoluteTranslation() const;

        const Vec2f & absoluteScaling() const;

        const Rect & bounds() const;

        const Rect & localBounds() const;

        /*Rect localHandleBounds() const;*/

        const Rect & handleBounds() const;

        const Rect & strokeBounds() const;

        Vec2f position() const;

        Vec2f pivot() const;

        bool isVisible() const;


        void setStrokeJoin(StrokeJoin _join);

        void setStrokeCap(StrokeCap _cap);

        void setMiterLimit(Float _limit);

        void setStrokeWidth(Float _width);

        void setStroke(const ColorRGBA & _color);

        void setStroke(const stick::String & _svgName);

        void setDashArray(const DashArray & _arr);

        void setDashOffset(Float _f);

        void setStrokeScaling(bool _b);

        void setNoStroke();

        void removeStroke();

        void setNoFill();

        void setFill(const ColorRGBA & _color);

        void setFill(const stick::String & _svgName);

        void setFill(const LinearGradient & _gradient);

        void setRemeshOnTransformChange(bool _b);

        void removeFill();

        void setWindingRule(WindingRule _rule);

        StrokeJoin strokeJoin() const;

        StrokeCap strokeCap() const;

        Float fillOpacity() const;

        Float strokeOpacity() const;

        Float miterLimit() const;

        Float strokeWidth() const;

        bool remeshOnTransformChange() const;

        const DashArray & dashArray() const;

        Float dashOffset() const;

        WindingRule windingRule() const;

        bool isScalingStroke() const;

        Paint fill() const;

        Paint stroke() const;

        bool hasStroke() const;

        bool hasFill() const;

        //clones this item and adds it ontop of it
        //in the DOM.
        Item clone() const;

        Document document() const;

        EntityType itemType() const;


        void markAbsoluteTransformDirty();

        void markFillGeometryDirty();

        void markStrokeGeometryDirty();

        void markGeometryDirty(bool _bMarkLengthDirty);

        void markBoundsDirty(bool _bNotifyParent);

        void markStrokeBoundsDirty(bool _bNotifyParent);

        void markFillBoundsDirty(bool _bNotifyParent);


        static Mat3f strokeTransform(const Mat3f * _transform, Float _strokeWidth, bool _bIsScalingStroke);


        template<class C>
        stick::Maybe<typename C::ValueType &> findComponent() const
        {
            Item i(*this);
            while (i.isValid())
            {
                auto maybe = i.maybe<C>();
                if (maybe)
                    return maybe;
                i = i.parent();
            }
            return stick::Maybe<typename C::ValueType &>();
        }

        static void addDefaultComponents(Item _item, Document * _doc);

    protected:

        void recursivePostTransform(bool _bIncludesScaling);

        void removeFromParent();

        void removeImpl(bool _bRemoveFromParent);

        Vec2f strokePadding(Float _strokeWidth, const Mat3f & _strokeMat) const;

        void decomposeIfNeeded() const;

        void decomposeAbsoluteIfNeeded() const;

        struct BoundsResult
        {
            bool bEmpty;
            Rect rect;
        };

        BoundsResult computeBounds(const Mat3f * _transform, BoundsType _type, bool _bAbsolute) const;
    };
}

#endif //PAPER_ITEM_HPP
