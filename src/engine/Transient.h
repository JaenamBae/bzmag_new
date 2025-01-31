#ifndef BZMAG_ENGINE_TRANSIENT_H_
#define BZMAG_ENGINE_TRANSIENT_H_

#include "platform.h"
#include "core/node.h"
#include "core/primitivetype.h"
#include <string>

namespace bzmag
{
    namespace engine
    {
        class Expression;
        class ENGINELIBRARY_API Transient : public Node
        {
        public:
            Transient();
            virtual ~Transient();

            DECLARE_CLASS(Transient, Node);

            const String& getTimeStep() const;
            void setTimeStep(const String& time_step);
            const String& getStopTime() const;
            void setStopTime(const String& stop_time);

            float64 evalTimeStep();
            float64 evalStopTime();

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        protected:
            virtual void clearBelongings() override;

        public:
            static void bindMethod();
            static void bindProperty();

        private:
            Ref<Expression> time_step_;
            Ref<Expression> stop_time_;
        };
    }
}

#endif //BZMAG_ENGINE_TRANSIENT_H_