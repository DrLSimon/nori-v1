#include <nori/shape.h>
#include <nori/bsdf.h>
#include <nori/emitter.h>

NORI_NAMESPACE_BEGIN
class Plane : public Shape {
public:
    Plane(const PropertyList &props){
    }

    void activate() {
        if (!m_bsdf) {
            /* If no material was assigned, instantiate a diffuse BRDF */
            m_bsdf = static_cast<BSDF *>(
                    NoriObjectFactory::createInstance("diffuse", PropertyList()));
        }
    }

    bool rayIntersect(Ray3f &_ray, Intersection &its, bool shadowRay = false) const {
        Normal3f n(0,1,0);
        Ray3f ray (_ray);
        float denominator = n.dot(ray.d); 
        if (denominator == 0.) 
            return false;
        float t = -n.dot(ray.o) / denominator;
        if (t >= _ray.mint and t <= _ray.maxt) {
            if (shadowRay)
                return true;
            _ray.maxt = its.t = t;
            its.p = _ray(its.t);
            its.uv = Point2f(its.p.x(),its.p.z()); 
            its.bsdf = getBSDF();
            its.emitter = getEmitter();
            its.geoFrame = its.shFrame = Frame(n);
            return true;
        }
        return false;
    }

    /// Return a pointer to an attached area emitter instance (const version)
    const Emitter *getEmitter() const { return m_emitter; }

    /// Return a pointer to the BSDF associated with this mesh
    const BSDF *getBSDF() const { return m_bsdf; }

    /// Register a child object (e.g. a BSDF) with the mesh
    void addChild(NoriObject *obj) {
        switch (obj->getClassType()) {
            case EBSDF:
                if (m_bsdf)
                    throw NoriException(
                        "Plane: tried to register multiple BSDF instances!");
                m_bsdf = static_cast<BSDF *>(obj);
                break;

            case EEmitter: {
                    Emitter *emitter = static_cast<Emitter *>(obj);
                    if (m_emitter)
                        throw NoriException(
                            "Plane: tried to register multiple Emitter instances!");
                    m_emitter = emitter;
                }
                break;

            default:
                throw NoriException("Plane::addChild(<%s>) is not supported!",
                                    classTypeName(obj->getClassType()));
        }
    }

    std::string toString() const {
        return tfm::format("Plane[\n"
                "emitter = %s\n"
                "bsdf = %s\n"
                "]", 
                (m_emitter) ? indent(m_emitter->toString()) : std::string("null"), 
                (m_bsdf) ? indent(m_bsdf->toString()) : std::string("null")
                );
    }
private:
    BSDF         *m_bsdf = nullptr;      ///< BSDF of the surface
    Emitter    *m_emitter = nullptr;     ///< Associated emitter, if any
};

NORI_REGISTER_CLASS(Plane, "plane");
NORI_NAMESPACE_END

