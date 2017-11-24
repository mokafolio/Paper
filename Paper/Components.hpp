#ifndef PAPER_COMPONENTS_HPP
#define PAPER_COMPONENTS_HPP

#include <Stick/TypeList.hpp>
#include <Brick/Component.hpp>
#include <Brick/Hub.hpp>
#include <Paper/Constants.hpp>

namespace paper
{
    class Document;
    class Item;
    class Paint;
    using ItemArray = stick::DynamicArray<Item>;

    namespace comps
    {
        //@TODO: I think we can group a lot of these together in the future to simplify
        //default components and possibly get a speed boost by adding the ones together
        //that are usually accessed together.
        using ItemType = brick::Component<ComponentName("ItemType"), EntityType>;
        using HubPointer = brick::Component<ComponentName("HubPointer"), brick::Hub *>;
        using Doc = brick::Component<ComponentName("Doc"), Document>;
        using Parent = brick::Component<ComponentName("Parent"), Item>;
        using Name = brick::Component<ComponentName("Name"), stick::String>;
        using Children = brick::Component<ComponentName("Children"), ItemArray>;
        using Transform = brick::Component<ComponentName("Transform"), Mat3f>;
        using AbsoluteTransform = brick::Component<ComponentName("AbsoluteTransform"), Mat3f>;
        using AbsoluteTransformDirtyFlag = brick::Component<ComponentName("AbsoluteTransformDirtyFlag"), bool>;
        using Fill = brick::Component<ComponentName("Fill"), Paint>;
        using Stroke = brick::Component<ComponentName("Stroke"), Paint>;
        using StrokeWidth = brick::Component<ComponentName("StrokeWidth"), Float>;
        using StrokeJoin = brick::Component<ComponentName("StrokeJoin"), StrokeJoin>;
        using StrokeCap = brick::Component<ComponentName("StrokeCap"), StrokeCap>;
        using ScalingStrokeFlag = brick::Component<ComponentName("ScalingStrokeFlag"), bool>;
        using MiterLimit = brick::Component<ComponentName("MiterLimit"), Float>;
        using DashArray = brick::Component<ComponentName("DashArray"), DashArray>;
        using DashOffset = brick::Component<ComponentName("DashOffset"), Float>;
        using WindingRule = brick::Component<ComponentName("WindingRule"), WindingRule>;
        using StrokeGeometryDirtyFlag = brick::Component<ComponentName("StrokeGeometryDirtyFlag"), bool>;
        using FillGeometryDirtyFlag = brick::Component<ComponentName("FillGeometryDirtyFlag"), bool>;
        using BoundsGeometryDirtyFlag = brick::Component<ComponentName("BoundsGeometryDirtyFlag"), bool>;
        using VisibilityFlag = brick::Component<ComponentName("VisibilityFlag"), bool>;
        using RemeshOnTransformChange = brick::Component<ComponentName("RemeshOnTransformChange"), bool>;

        struct BoundsData
        {
            bool bDirty;
            Rect bounds;
        };

        using HandleBounds = brick::Component<ComponentName("HandleBounds"), BoundsData>;
        using StrokeBounds = brick::Component<ComponentName("StrokeBounds"), BoundsData>;
        using Bounds = brick::Component<ComponentName("Bounds"), BoundsData>;
        using LocalBounds = brick::Component<ComponentName("LocalBounds"), BoundsData>;
        using Pivot = brick::Component<ComponentName("Pivot"), Vec2f>;

        struct DecomposedData
        {
            Vec2f translation;
            Float rotation;
            Vec2f scaling;
        };

        using DecomposedTransform = brick::Component<ComponentName("DecomposedTransform"), DecomposedData>;
        using AbsoluteDecomposedTransform = brick::Component<ComponentName("AbsoluteDecomposedTransform"), DecomposedData>;

        //document specific components
        using DocumentSize = brick::Component<ComponentName("DocumentSize"), Vec2f>;

        //group specific components
        using ClippedFlag = brick::Component<ComponentName("ClippedFlag"), bool>;

        //Path specific components
        using Segments = brick::Component<ComponentName("Segments"), SegmentArray>;
        using Curves = brick::Component<ComponentName("Curves"), CurveArray>;
        using ClosedFlag = brick::Component<ComponentName("ClosedFlag"), bool>;
        struct PathLengthData
        {
            bool bDirty;
            Float length;
        };
        using PathLength = brick::Component<ComponentName("PathLength"), PathLengthData>;

        //type list of all components. Because C++ is shit, we need to manually maintain it
        //as there is no decent way to my knowledge to automatically generate this at compile time.
        using ComponentTypeList = typename stick::MakeTypeList <
                                  ItemType,
                                  HubPointer,
                                  Doc,
                                  Parent,
                                  Name,
                                  Children,
                                  Transform,
                                  AbsoluteTransform,
                                  AbsoluteTransformDirtyFlag,
                                  Fill,
                                  Stroke,
                                  StrokeWidth,
                                  StrokeJoin,
                                  StrokeCap,
                                  ScalingStrokeFlag,
                                  MiterLimit,
                                  DashArray,
                                  DashOffset,
                                  WindingRule,
                                  StrokeGeometryDirtyFlag,
                                  FillGeometryDirtyFlag,
                                  BoundsGeometryDirtyFlag,
                                  VisibilityFlag,
                                  RemeshOnTransformChange,
                                  HandleBounds,
                                  StrokeBounds,
                                  Bounds,
                                  LocalBounds,
                                  Pivot,
                                  DecomposedTransform,
                                  AbsoluteDecomposedTransform,
                                  DocumentSize,
                                  ClippedFlag,
                                  Segments,
                                  Curves,
                                  ClosedFlag,
                                  PathLength
                                  >::List;
    }
}

#endif //PAPER_COMPONENTS_HPP
