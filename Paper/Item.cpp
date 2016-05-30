#include <Paper/Item.hpp>
#include <Paper/Components.hpp>
#include <Paper/Path.hpp>
#include <Paper/Group.hpp>
#include <Paper/Document.hpp>
#include <Paper/Private/BooleanOperations.hpp> //for removing the mono curve component in markGeometryDirty
#include <Crunch/MatrixFunc.hpp>

#include <Crunch/StringConversion.hpp>

namespace paper
{
    Item::Item()
    {

    }

    Item::Item(const brick::Entity & _e) :
        brick::Entity(_e)
    {

    }

    void Item::addChild(brick::Entity _e)
    {
        STICK_ASSERT(_e.isValid());
        STICK_ASSERT(_e.get<comps::ItemType>() == EntityType::Path ||
                     _e.get<comps::ItemType>() == EntityType::Group);

        if (!hasComponent<comps::Children>())
            set<comps::Children>(EntityArray());

        //this is a compound path
        if (get<comps::ItemType>() == EntityType::Path)
        {
            //assert that the child we add is a path too
            STICK_ASSERT(_e.get<comps::ItemType>() == EntityType::Path);

            //for non zero winding rule we adjust the direction of the added path if needed
            if (windingRule() == WindingRule::NonZero)
            {
                Path p(_e);
                Path tp(*this);
                p.setClockwise(!tp.isClockwise());
            }
        }

        //possibly remove from previous parent
        Item e2 = _e;
        e2.removeFromParent();
        get<comps::Children>().append(e2);
        _e.set<comps::Parent>(*this);

        //the bounds are dirty now
        markBoundsDirty(true);
    }

    void Item::insertAbove(brick::Entity _e)
    {
        removeFromParent();

        STICK_ASSERT(_e.hasComponent<comps::Parent>());
        Item p = _e.get<comps::Parent>();
        STICK_ASSERT(p.isValid());
        auto & children = p.get<comps::Children>();
        auto it = stick::find(children.begin(), children.end(), _e);
        STICK_ASSERT(it != children.end());
        children.insert(it + 1, *this);
        set<comps::Parent>(p);

        //the new parent bounds are dirty now
        p.markBoundsDirty(true);
    }

    void Item::insertBelow(brick::Entity _e)
    {
        removeFromParent();

        STICK_ASSERT(_e.hasComponent<comps::Parent>());
        Item p = _e.get<comps::Parent>();
        STICK_ASSERT(p.isValid());
        auto & children = p.get<comps::Children>();
        auto it = stick::find(children.begin(), children.end(), _e);
        STICK_ASSERT(it != children.end());
        children.insert(it, *this);
        set<comps::Parent>(p);

        //the new parent bounds are dirty now
        p.markBoundsDirty(true);
    }

    void Item::sendToFront()
    {
        STICK_ASSERT(hasComponent<comps::Parent>());
        Item p = get<comps::Parent>();
        STICK_ASSERT(p.isValid());

        removeFromParent();

        p.get<comps::Children>().append(*this);
        set<comps::Parent>(p);

        //the new parent bounds are dirty now
        p.markBoundsDirty(true);
    }

    void Item::sendToBack()
    {
        STICK_ASSERT(hasComponent<comps::Parent>());
        Item p = get<comps::Parent>();
        STICK_ASSERT(p.isValid());

        removeFromParent();

        p.get<comps::Children>().insert(p.get<comps::Children>().begin(), *this);
        set<comps::Parent>(p);

        //the new parent bounds are dirty now
        p.markBoundsDirty(true);
    }

    void Item::remove()
    {
        STICK_ASSERT(isValid());
        removeImpl(true);
    }

    void Item::removeImpl(bool _bRemoveFromParent)
    {
        removeChildren();

        if (_bRemoveFromParent)
            removeFromParent();
        destroy();
    }

    void Item::removeChildren()
    {
        if (hasComponent<comps::Children>())
        {
            auto & cs = get<comps::Children>();
            for (Item child : cs)
                child.removeImpl(false);
            cs.clear();
        }
    }

    const EntityArray & Item::children() const
    {
        if (!hasComponent<comps::Children>())
        {
            static EntityArray s_proxy;
            return s_proxy;
        }
        return get<comps::Children>();
    }

    const stick::String & Item::name() const
    {
        return get<comps::Name>();
    }

    brick::Entity Item::parent() const
    {
        if (hasComponent<comps::Parent>())
            return get<comps::Parent>();
        else
            return brick::Entity();
    }

    void Item::removeFromParent()
    {
        if (hasComponent<comps::Parent>())
        {
            Item p = get<comps::Parent>();
            if (p.isValid())
            {
                auto & children = p.get<comps::Children>();
                auto it = stick::find(children.begin(), children.end(), *this);
                STICK_ASSERT(it != children.end());
                children.remove(it);
                set<comps::Parent>(brick::Entity());
                p.markBoundsDirty(true);
            }
        }
    }

    void Item::setTransform(const Mat3f & _transform)
    {
        set<comps::Transform>(_transform);
        markBoundsDirty(true);
        markAbsoluteTransformDirty();
    }

    void Item::setPosition(const Vec2f & _position)
    {
        translateTransform(_position - pivot());
    }

    void Item::setPivot(const Vec2f & _pivot)
    {
        set<comps::Pivot>(_pivot);
    }

    void Item::setName(const stick::String & _name)
    {
        set<comps::Name>(_name);
    }

    void Item::translateTransform(Float _x, Float _y)
    {
        translateTransform(Vec2f(_x, _y));
    }

    void Item::translateTransform(const Vec2f & _translation)
    {
        transform(Mat3f::translation2D(_translation));
    }

    void Item::scaleTransform(Float _scale)
    {
        scaleTransform(Vec2f(_scale, _scale));
    }

    void Item::scaleTransform(Float _scaleX, Float _scaleY)
    {
        scaleTransform(Vec2f(_scaleX, _scaleY));
    }

    void Item::scaleTransform(const Vec2f & _scale)
    {
        scaleTransform(_scale, pivot());
    }

    void Item::scaleTransform(const Vec2f & _scale, const Vec2f & _center)
    {
        Mat3f mat = Mat3f::translation2D(_center);
        mat.scale2D(_scale);
        mat.translate2D(-_center);
        transform(mat);
    }

    void Item::rotateTransform(Float _radians)
    {
        rotateTransform(_radians, pivot());
    }

    void Item::rotateTransform(Float _radians, const Vec2f & _point)
    {
        Mat3f mat = Mat3f::translation2D(_point);
        mat.rotate2D(_radians);
        mat.translate2D(-_point);
        transform(mat);
    }

    void Item::transform(const Mat3f & _matrix)
    {
        set<comps::Transform>(_matrix * transform());
        markBoundsDirty(true);
        markAbsoluteTransformDirty();
    }

    void Item::translate(Float _x, Float _y)
    {
        translate(Vec2f(_x, _y));
    }

    void Item::translate(const Vec2f & _translation)
    {
        applyTransform(Mat3f::translation2D(_translation));
    }

    void Item::scale(Float _scale)
    {
        scale(Vec2f(_scale), pivot());
    }

    void Item::scale(Float _scaleX, Float _scaleY)
    {
        scale(Vec2f(_scaleX, _scaleY), pivot());
    }

    void Item::scale(const Vec2f & _scale)
    {
        scale(_scale, pivot());
    }

    void Item::scale(const Vec2f & _scale, const Vec2f & _center)
    {
        Mat3f mat = Mat3f::translation2D(_center);
        mat.scale2D(_scale);
        mat.translate2D(-_center);
        applyTransform(mat);
    }

    void Item::rotate(Float _radians)
    {
        rotate(_radians, pivot());
    }

    void Item::rotate(Float _radians, const Vec2f & _point)
    {
        Mat3f mat = Mat3f::translation2D(_point);
        mat.rotate2D(_radians);
        mat.translate2D(-_point);
        applyTransform(mat);
    }

    void Item::applyTransform(const Mat3f & _transform, bool _bNotifyParent)
    {
        auto itemType = get<comps::ItemType>();
        if (itemType == EntityType::Path)
        {
            Path p(*this);
            p.applyTransform(_transform);
            markGeometryDirty();
            markBoundsDirty(_bNotifyParent);
        }

        if (hasComponent<comps::Children>())
        {
            auto & cs = children();
            for (Item c : cs)
                c.applyTransform(_transform, false);
        }
    }

    const Mat3f & Item::transform() const
    {
        if (hasComponent<comps::Transform>())
            return get<comps::Transform>();

        static Mat3f s_proxy = Mat3f::identity();
        return s_proxy;
    }

    const Mat3f & Item::absoluteTransform() const
    {
        if (hasComponent<comps::AbsoluteTransformDirtyFlag>())
        {
            if (get<comps::AbsoluteTransformDirtyFlag>())
            {
                Item p = parent();
                if (!p.isValid())
                {
                    const_cast<Item *>(this)->set<comps::AbsoluteTransform>(transform());
                }
                else
                {
                    const_cast<Item *>(this)->set<comps::AbsoluteTransform>(p.absoluteTransform() * transform());
                }
                const_cast<Item *>(this)->set<comps::AbsoluteTransformDirtyFlag>(false);
            }

            return get<comps::AbsoluteTransform>();
        }

        // return the proxy identity
        return transform();
    }

    void Item::decomposeIfNeeded() const
    {
        if (hasComponent<comps::DecomposedTransform>())
            return;
        Float rotation;
        Vec2f scaling, translation;
        crunch::decompose(transform(), translation, rotation, scaling);
        const_cast<Item*>(this)->set<comps::DecomposedTransform>((comps::DecomposedData){translation, rotation, scaling});
    }

    stick::Float32 Item::rotation() const
    {
        decomposeIfNeeded();
        return get<comps::DecomposedTransform>().rotation;
    }

    const Vec2f & Item::translation() const
    {
        decomposeIfNeeded();
        return get<comps::DecomposedTransform>().translation;
    }

    const Vec2f & Item::scaling() const
    {
        decomposeIfNeeded();
        return get<comps::DecomposedTransform>().scaling;
    }

    Vec2f Item::absoluteScaling() const
    {
        decomposeIfNeeded();
        if (parent().isValid())
            return Item(parent()).absoluteScaling() + scaling();
    }

    Vec2f Item::absoluteTranslation() const
    {
        decomposeIfNeeded();
        if (parent().isValid())
            return Item(parent()).absoluteTranslation() + translation();
    }

    Vec2f Item::absoluteRotation() const
    {
        decomposeIfNeeded();
        if (parent().isValid())
            return Item(parent()).absoluteRotation() + rotation();
    }

    Vec2f Item::strokePadding(Float _strokeWidth, const Mat3f & _mat) const
    {
        // If a matrix is provided, we need to rotate the stroke circle
        // and calculate the bounding box of the resulting rotated elipse:
        // Get rotated hor and ver vectors, and determine rotation angle
        // and elipse values from them:
        Mat3f shiftless(_mat[0], _mat[1], crunch::Vec3f(0, 0, 1));
        //Mat3f shiftless = _mat;
        Vec2f hor = shiftless * Vec2f(_strokeWidth, 0.0f);
        Vec2f vert = shiftless * Vec2f(0.0f, _strokeWidth);
        Float phi = atan2(hor.y, hor.x);
        Float hlen = crunch::length(hor);
        Float vlen = crunch::length(vert);
        // Formula for rotated ellipses:
        // x = cx + a*cos(t)*cos(phi) - b*sin(t)*sin(phi)
        // y = cy + b*sin(t)*cos(phi) + a*cos(t)*sin(phi)
        // Derivates (by Wolfram Alpha):
        // derivative of x = cx + a*cos(t)*cos(phi) - b*sin(t)*sin(phi)
        // dx/dt = a sin(t) cos(phi) + b cos(t) sin(phi) = 0
        // derivative of y = cy + b*sin(t)*cos(phi) + a*cos(t)*sin(phi)
        // dy/dt = b cos(t) cos(phi) - a sin(t) sin(phi) = 0
        // This can be simplified to:
        // tan(t) = -b * tan(phi) / a // x
        // tan(t) =  b * cot(phi) / a // y
        // Solving for t gives:
        // t = pi * n - arctan(b * tan(phi) / a) // x
        // t = pi * n + arctan(b * cot(phi) / a)
        //   = pi * n + arctan(b / tan(phi) / a) // y
        Float s = std::sin(phi);
        Float c = std::cos(phi);
        Float t = std::tan(phi);
        Float tx = std::atan2(vlen * t, hlen);
        Float ty = std::atan2(vlen, t * hlen);
        return crunch::abs(Vec2f(hlen * std::cos(tx) * c + vlen * std::sin(tx) * s,
                                 vlen * std::sin(ty) * c + hlen * std::cos(ty) * s));
    }

    Mat3f Item::strokeTransform(const Mat3f * _transform, Float _strokeWidth, bool _bIsScalingStroke)
    {
        Float hsw = _strokeWidth * 0.5;
        Mat3f tmp;
        if (_bIsScalingStroke)
        {
            tmp = Mat3f::identity() * hsw;
        }
        else
        {
            STICK_ASSERT(_transform);
            tmp = crunch::inverse(*_transform);
            tmp.scale2D(hsw);
        }

        //take out the translation
        //return Mat3f(tmp[0], tmp[1], crunch::Vec3f(0, 0, 1));
        return tmp;
    }

    Item::BoundsResult Item::computeBounds(const Mat3f * _transform, BoundsType _type, bool _bAbsolute) const
    {
        STICK_ASSERT(hasComponent<comps::ItemType>());
        auto itemType = get<comps::ItemType>();
        BoundsResult ret, tmp;
        ret = {true, Rect(0, 0, 0, 0)};
        if (itemType == EntityType::Path)
        {
            if (_type == BoundsType::Fill)
                ret = Path(*this).computeBounds(_bAbsolute ? &absoluteTransform() : _transform);
            else if (_type == BoundsType::Stroke)
                ret = Path(*this).computeStrokeBounds(_bAbsolute ? &absoluteTransform() : _transform);
            else if (_type == BoundsType::Handle)
                ret = Path(*this).computeHandleBounds(_bAbsolute ? &absoluteTransform() : _transform);
        }
        else if (itemType == EntityType::Group)
        {
            Group grp(*this);
            if (grp.isClipped())
            {
                if (grp.children().count())
                    return Item(grp.children()[0]).computeBounds(_transform, _type, _bAbsolute);
                else
                    return {true, Rect(0, 0, 0, 0)};
            }
        }

        // simply merge the children bounds recursively
        auto & cs = children();

        if (itemType != EntityType::Path && !cs.count())
            return {true, Rect(0, 0, 0, 0)};
        else if (itemType == EntityType::Path)
        {
            if (ret.bEmpty)
                return ret;
        }

        Mat3f tmpMat;
        auto it = cs.begin();
        for (; it !=  cs.end(); ++it)
        {
            Item item(*it);

            if (_bAbsolute)
            {
                tmp = item.computeBounds(&item.absoluteTransform(), _type, true);
            }
            else
            {
                tmpMat = _transform ? *_transform * item.transform() : item.transform();
                tmp = item.computeBounds(&tmpMat, _type, false);
            }

            if (tmp.bEmpty)
                continue;

            if (ret.bEmpty)
            {
                ret = tmp;
            }
            else
            {
                if (!tmp.bEmpty)
                    ret.rect = crunch::merge(ret.rect, tmp.rect);
            }
        }

        return ret;
    }

    template<class C>
    void removeComponentFromChildren(Item _item)
    {
        if (!_item.hasComponent<comps::Children>())
            return;
        auto & children = _item.get<comps::Children>();
        for (auto & c : children)
        {
            c.removeComponent<C>();
            removeComponentFromChildren<C>(Item(c));
        }
    }

    void Item::setVisible(bool _b)
    {
        set<comps::VisibilityFlag>(_b);
    }

    void Item::setStrokeJoin(StrokeJoin _join)
    {
        set<comps::StrokeJoin>(_join);
        removeComponentFromChildren<comps::StrokeJoin>(*this);
        markStrokeBoundsDirty(true);
        markStrokeGeometryDirty();
    }

    void Item::setStrokeCap(StrokeCap _cap)
    {
        set<comps::StrokeCap>(_cap);
        removeComponentFromChildren<comps::StrokeCap>(*this);
        markStrokeBoundsDirty(true);
        markStrokeGeometryDirty();
    }

    void Item::setStrokeScaling(bool _b)
    {
        set<comps::ScalingStrokeFlag>(_b);
        markStrokeBoundsDirty(true);
        markStrokeGeometryDirty();
    }

    void Item::setMiterLimit(Float _limit)
    {
        set<comps::MiterLimit>(_limit);
        removeComponentFromChildren<comps::MiterLimit>(*this);
        markStrokeBoundsDirty(true);
        markStrokeGeometryDirty();
    }

    void Item::setStrokeWidth(Float _width)
    {
        set<comps::StrokeWidth>(_width);
        removeComponentFromChildren<comps::StrokeWidth>(*this);
        markStrokeBoundsDirty(true);
        markStrokeGeometryDirty();
    }

    void Item::setDashArray(const DashArray & _arr)
    {
        set<comps::DashArray>(_arr);
        removeComponentFromChildren<comps::DashArray>(*this);
        markStrokeBoundsDirty(true);
        markStrokeGeometryDirty();
    }

    void Item::setDashOffset(Float _offset)
    {
        set<comps::DashOffset>(_offset);
        removeComponentFromChildren<comps::DashOffset>(*this);
        markStrokeGeometryDirty();
    }

    void Item::setStroke(const ColorRGBA & _color)
    {
        if (!hasStroke())
        {
            markStrokeGeometryDirty();
            markStrokeBoundsDirty(true);
        }

        set<comps::Stroke>(_color);
        removeComponentFromChildren<comps::Stroke>(*this);
    }

    void Item::removeStroke()
    {
        removeComponent<comps::Stroke>();
        markStrokeBoundsDirty(true);
    }

    void Item::setFill(const ColorRGBA & _color)
    {
        set<comps::Fill>(_color);
        removeComponentFromChildren<comps::Fill>(*this);
    }

    void Item::removeFill()
    {
        removeComponent<comps::Fill>();
    }

    void Item::setWindingRule(WindingRule _rule)
    {
        set<comps::WindingRule>(_rule);
    }

    const ColorRGBA & Item::fill() const
    {
        auto ret = findComponent<comps::Fill>();
        if (ret)
        {
            return *ret;
        }
        static ColorRGBA s_colorProxy(1.0, 1.0, 1.0, 1.0);
        return s_colorProxy;
    }

    const ColorRGBA & Item::stroke() const
    {
        auto ret = findComponent<comps::Stroke>();
        if (ret)
        {
            return *ret;
        }
        static ColorRGBA s_colorProxy(0.0, 0.0, 0.0, 1.0);
        return s_colorProxy;
    }

    const DashArray & Item::dashArray() const
    {
        auto ret = findComponent<comps::DashArray>();
        if (ret)
        {
            return *ret;
        }
        static DashArray s_proxy;
        return s_proxy;
    }

    Float Item::dashOffset() const
    {
        auto ret = findComponent<comps::DashOffset>();
        if (ret)
        {
            return *ret;
        }
        return 0.0;
    }

    bool Item::hasStroke() const
    {
        return (bool)findComponent<comps::Stroke>();
    }

    bool Item::hasFill() const
    {
        return (bool)findComponent<comps::Fill>();
    }

    StrokeJoin Item::strokeJoin() const
    {
        auto ret = findComponent<comps::StrokeJoin>();
        if (ret)
        {
            return *ret;
        }
        return StrokeJoin::Miter;
    }

    StrokeCap Item::strokeCap() const
    {
        auto ret = findComponent<comps::StrokeCap>();
        if (ret)
        {
            return *ret;
        }
        return StrokeCap::Butt;
    }

    Float Item::strokeWidth() const
    {
        auto ret = findComponent<comps::StrokeWidth>();
        if (ret)
        {
            return *ret;
        }
        return (Float)1.0;
    }

    Float Item::miterLimit() const
    {
        auto ret = findComponent<comps::MiterLimit>();
        if (ret)
        {
            return *ret;
        }
        return (Float)10.0;
    }

    bool Item::isScalingStroke() const
    {
        auto ret = findComponent<comps::ScalingStrokeFlag>();
        if (ret)
        {
            return *ret;
        }
        return true;
    }

    WindingRule Item::windingRule() const
    {
        auto ret = findComponent<comps::WindingRule>();
        if (ret)
        {
            return *ret;
        }
        return WindingRule::EvenOdd;
    }

    const Rect & Item::bounds() const
    {
        auto & b = const_cast<Item *>(this)->get<comps::Bounds>();
        if (b.bDirty)
        {
            b.bounds = computeBounds(nullptr, BoundsType::Fill, true).rect;
            b.bDirty = false;
        }
        return b.bounds;
    }

    const Rect & Item::localBounds() const
    {
        auto & b = const_cast<Item *>(this)->get<comps::LocalBounds>();
        if (b.bDirty)
        {
            b.bounds = computeBounds(nullptr, BoundsType::Fill, false).rect;
            b.bDirty = false;
        }
        return b.bounds;
    }

    const Rect & Item::strokeBounds() const
    {
        auto & b = const_cast<Item *>(this)->get<comps::StrokeBounds>();
        if (b.bDirty)
        {
            b.bounds = computeBounds(nullptr, BoundsType::Stroke, true).rect;
            b.bDirty = false;
        }
        return b.bounds;
    }

    const Rect & Item::handleBounds() const
    {
        auto & b = const_cast<Item *>(this)->get<comps::HandleBounds>();
        if (b.bDirty)
        {
            b.bounds = computeBounds(nullptr, BoundsType::Handle, true).rect;
            b.bDirty = false;
        }
        return b.bounds;
    }

    Vec2f Item::position() const
    {
        const Rect & b = bounds();
        return b.center();
    }

    Vec2f Item::pivot() const
    {
        if (hasComponent<comps::Pivot>())
            return get<comps::Pivot>();
        return position();
    }

    bool Item::isVisible() const
    {
        if (hasComponent<comps::VisibilityFlag>())
            return get<comps::VisibilityFlag>();
        return true;
    }

    void Item::markBoundsDirty(bool _bNotifyParent)
    {
        markStrokeBoundsDirty(false);
        markFillBoundsDirty(false);
        if (_bNotifyParent)
        {
            Item p = parent();
            if (p.isValid())
            {
                p.markBoundsDirty(true);
            }
        }
    }

    void Item::markStrokeBoundsDirty(bool _bNotifyParent)
    {
        auto & sbounds = get<comps::StrokeBounds>();
        sbounds.bDirty = true;
        if (_bNotifyParent)
        {
            Item p = parent();
            if (p.isValid())
            {
                p.markStrokeBoundsDirty(true);
            }
        }
    }

    void Item::markFillBoundsDirty(bool _bNotifyParent)
    {
        auto & bounds = get<comps::Bounds>();
        bounds.bDirty = true;
        auto & lbounds = get<comps::LocalBounds>();
        lbounds.bDirty = true;
        set<comps::BoundsGeometryDirtyFlag>(true);
        if (_bNotifyParent)
        {
            Item p = parent();
            if (p.isValid())
            {
                p.markFillBoundsDirty(true);
            }
        }
    }

    void Item::markAbsoluteTransformDirty()
    {
        set<comps::AbsoluteTransformDirtyFlag>(true);
        for (Item c : children())
            c.markAbsoluteTransformDirty();
    }

    void Item::markFillGeometryDirty()
    {
        set<comps::FillGeometryDirtyFlag>(true);
        if (hasComponent<detail::comps::MonoCurves>())
            removeComponent<detail::comps::MonoCurves>();
    }

    void Item::markStrokeGeometryDirty()
    {
        set<comps::StrokeGeometryDirtyFlag>(true);
    }

    void Item::markGeometryDirty()
    {
        markFillGeometryDirty();
        markStrokeGeometryDirty();
    }

    static Item cloneGroup(const Item & _grp)
    {
        Group copy = _grp.cloneWithout<comps::Parent, comps::Children>();
        for (Item child : _grp.children())
        {
            copy.addChild(child.clone());
        }

        copy.insertAbove(_grp);
        return copy;
    }

    static Item clonePath(const Item & _path)
    {
        Path from(_path);
        Path copy = from.cloneWithout<comps::Parent, comps::Segments, comps::Curves>();
        STICK_ASSERT(!copy.hasComponent<comps::Parent>());
        STICK_ASSERT(!copy.hasComponent<comps::Segments>());
        STICK_ASSERT(!copy.hasComponent<comps::Curves>());
        copy.set<comps::Segments>(SegmentArray());
        copy.set<comps::Curves>(CurveArray());
        copy.set<comps::Children>(EntityArray());
        for (const auto & seg : from.segments())
        {
            copy.addSegment(seg.position(), seg.handleIn(), seg.handleOut());
        }

        if (from.isClosed())
        {
            copy.set<comps::ClosedFlag>(false);
            copy.closePath();
        }

        copy.insertAbove(_path);

        for (Item child : from.children())
        {
            copy.addChild(child.clone());
        }

        return copy;
    }

    Item Item::clone() const
    {
        if (get<comps::ItemType>() == EntityType::Group)
        {
            return cloneGroup(*this);
        }
        else if (get<comps::ItemType>() == EntityType::Path)
        {
            return clonePath(*this);
        }
        return Item();
    }

    Document Item::document() const
    {
        //the root of the item hierarchy is the document
        Item item(*this);
        while (item.parent().isValid())
        {
            item = item.parent();
        }
        STICK_ASSERT(item.get<comps::ItemType>() == EntityType::Document);
        return Document(item);
    }

    EntityType Item::itemType() const
    {
        return get<comps::ItemType>();
    }
}
