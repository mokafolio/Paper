#ifndef PAPER_PRIVATE_CONTAINERVIEW_HPP
#define PAPER_PRIVATE_CONTAINERVIEW_HPP

namespace paper
{
    namespace detail
    {
        template<class From, class To>
        struct Transformer;

        template<class T>
        struct Transformer<stick::UniquePtr<T>, T>
        {
            static T & transform(const stick::UniquePtr<T> & _from)
            {
                return *_from;
            };
        };

        template <bool Const, class ConstIter, class Iter, class TO>
        struct ContainerViewTraits;

        template<class ConstIter, class Iter, class TO>
        struct ContainerViewTraits<true, ConstIter, Iter, TO>
        {
            using IterType = ConstIter;
            using ValueType = const TO;
        };

        template<class ConstIter, class Iter, class TO>
        struct ContainerViewTraits<false, ConstIter, Iter, TO>
        {
            using IterType = Iter;
            using ValueType = TO;
        };

        template<bool Const, class CT, class TO>
        class ContainerView
        {
        public:
            using ContainerType = CT;
            using ContainerIter = typename CT::Iter;
            using ContainerConstIter = typename CT::ConstIter;
            using ContainerItemType = typename CT::ValueType;
            using Traits = ContainerViewTraits<Const, ContainerConstIter, ContainerIter, TO>;
            using ValueType = typename Traits::ValueType;
            using PointerType = ValueType*;
            using ReferenceType = ValueType&;
            using InternalIter = typename Traits::IterType;

            template<class VT>
            struct IterT
            {
                using ValueType = VT;

                IterT()
                {

                }

                IterT(const InternalIter & _it) :
                    m_it(_it)
                {

                }

                IterT(const IterT & _other) :
                    m_it(_other.m_it)
                {

                }

                bool operator == (const IterT & _other) const
                {
                    return m_it == _other.m_it;
                }

                bool operator != (const IterT & _other) const
                {
                    return m_it != _other.m_it;
                }

                ValueType & operator* () const
                {
                    return Transformer<ContainerItemType, ValueType>::transform(*m_it);
                }

                IterT & operator++()
                {
                    m_it++;
                    return *this;
                }

                IterT operator++(int)
                {
                    IterT ret(*this);
                    ++(*this);
                    return ret;
                }

                InternalIter m_it;
            };

            using Iter = IterT<ValueType>;
            //using ConstIter = IterT<true, const To>;


            ContainerView()
            {
            }

            ContainerView(InternalIter _begin, InternalIter _end) :
                m_begin(_begin),
                m_end(_end)
            {

            }

            ContainerView(const ContainerView & _other) :
                m_begin(_other.m_begin),
                m_end(_other.m_end)
            {

            }

            Iter begin() const
            {
                return Iter(m_begin);
            }

            Iter end() const
            {
                return Iter(m_end);
            }

        private:

            InternalIter m_begin;
            InternalIter m_end;
        };
    }
}

#endif //PAPER_PRIVATE_CONTAINERVIEW_HPP
