#ifndef BZMAG_ENGINE_SOLUTION_SETUP_H_
#define BZMAG_ENGINE_SOLUTION_SETUP_H_

#include "platform.h"
#include "core/node.h"
#include "core/primitivetype.h"
#include <string>

namespace bzmag
{
    namespace engine
    {
        class Transient;
        class ENGINELIBRARY_API SolutionSetup : public Node
        {
        public:
            SolutionSetup();
            virtual ~SolutionSetup();

            DECLARE_CLASS(SolutionSetup, Node);

        public:
            void setAbsoluteTolerance(float64 tol);
            float64 getAbsoluteTolerance() const;

            void setRelativeTolerance(float64 tol);
            float64 getRelativeTolerance() const;

            void setMaxIteration(int32 iter);
            int32 getMaxIteration() const;

            void setSymmetrcyFactor(float64 factor);
            float64 getSymmetrcyFactor() const;

            void setLengthZ(float64 length);
            float64 getLengthZ() const;

            Transient* getTransientNode();

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        public:
            static void bindMethod();
            static void bindProperty();

        private:
            float64 tol_abs_ = 1e-6;
            float64 tol_rel_ = 1e-6;
            int32 iter_max_ = 20;

            float64 symmetry_factor_ = 1.0;
            float64 z_length_ = 1.0;

            bool save_all_steps_ = true;
            bool clean_results_ = false;

            std::string command = "-solve -v 3 -v2";

        };
    }
}

#endif // !BZMAG_ENGINE_SOLUTION_SETUP_H_
