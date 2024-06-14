//
// Created by root on 5/25/24.
//

#ifndef CXX_TEST_OPS_HPP
#define CXX_TEST_OPS_HPP


namespace INSTINCT_TRANSFORMER_NS::ops {

    static void ggml_compute_forward_sigmoid_f32(struct ggml_tensor * dst , const struct ggml_tensor * src0, int ith, int nth, void * userdata) {
        GGML_ASSERT(ggml_are_same_shape(src0, dst));

        GGML_ASSERT(dst->type == GGML_TYPE_F32);

        GGML_TENSOR_UNARY_OP_LOCALS

        // TODO: make this tunable
        float eps = 1e-6f;

        GGML_ASSERT(eps > 0.0f);

        // TODO: optimize
        for (int64_t i03 = 0; i03 < ne03; i03++) {
            for (int64_t i02 = 0; i02 < ne02; i02++) {
                for (int64_t i01 = ith; i01 < ne01; i01 += nth) {

                    for (int64_t i00 = 0; i00 < ne00; i00++) {
                        const float * x = (float *) ((char *) src0->data + i00*nb00 + i01*nb01 + i02*nb02 + i03*nb03);
                        auto * y = (float *) ((char *) dst->data + i00*nb0 + i01*nb1 + i02*nb2 + i03*nb3);
                        *y = 1 / (1 + expf(- *x));
                    }
                }
            }
        }
    }

    static void ggml_compute_forward_sigmoid(struct ggml_tensor * dst , const struct ggml_tensor * src, int ith, int nth, void * userdata) {
        switch (src->type) {
            case GGML_TYPE_F32:
            {
                ggml_compute_forward_sigmoid_f32(dst, src, ith, nth, userdata);
            } break;
            default:
            {
                GGML_ASSERT(false);
            } break;
        }
    }

}

#endif //CXX_TEST_OPS_HPP
