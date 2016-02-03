/*
 * Tree structure use to store genes.
 */
#ifndef featureTree_hh
#define featureTree_hh
#include <assert.h>
#include <functional>
#include "gxf.hh"
#include <ostream>
#include "remapStatus.hh"

// FIXME: add the substituted target stuff caused this to step into hacky

/* Remap status attribute name */
extern const string REMAP_STATUS_ATTR;

/* Attribute name used for original id before remap */
extern const string REMAP_ORIGINAL_ID_ATTR;

/* Attribute name used for original location before remap */
extern const string REMAP_ORIGINAL_LOCATION_ATTR;

/* Attribute name for count of mappings, set on transcripts or genes */
extern const string REMAP_NUM_MAPPINGS_ATTR;

/* Attribute name for target of mapping */
extern const string REMAP_TARGET_STATUS_ATTR;

/* Attribute indicating target gene was substituted due to not being
* able to map gene. Value version   */
extern const string REMAP_SUBSTITUTED_MISSING_TARGET_ATTR;

class FeatureNode;
/* Vector of FeatureNode objects */
typedef vector<FeatureNode*> FeatureNodeVector;


/**
 * Tree container for a GxfFeature object and children
 */
class FeatureNode {
    public:
    GxfFeature* fFeature;
    FeatureNode* fParent;
    FeatureNodeVector fChildren;
    RemapStatus fRemapStatus;
    TargetStatus fTargetStatus;
    int fNumMappings;    // Number of location feature was mapped too.  Not set for all node types.

    private:
    int getTranscriptExonSize() const;
    int getOverlapAmount(const FeatureNode* other) const;
    int countExonOverlap(const FeatureNode* exon1,
                         const FeatureNode* trans2) const;
    float getMaxTranscriptSimilarity(const FeatureNode* gene2,
                                     const FeatureNode* trans1,
                                     bool manualOnlyTranscripts) const;

    public:
    FeatureNode(GxfFeature* feature):
        fFeature(feature),
        fParent(NULL),
        fRemapStatus(REMAP_STATUS_NONE),
        fTargetStatus(TARGET_STATUS_NA),
        fNumMappings(0) {
    }

    ~FeatureNode() {
        delete fFeature;
        for (size_t i = 0; i < fChildren.size(); i++) {
            delete fChildren[i];
        }
    }

    /* is this a gene? */
    bool isGene() const {
        return (fFeature->fType == GxfFeature::GENE);
    }

    /* is this a transcript? */
    bool isTranscript() const {
        return (fFeature->fType == GxfFeature::TRANSCRIPT);
    }

    /* is this an exon? */
    bool isExon() const {
        return (fFeature->fType == GxfFeature::EXON);
    }

    /* is this a gene or transcript */
    bool isGeneOrTranscript() const {
        return isGene() or isTranscript();
    }

    /* get the id based on feature type, or empty string if it doesn't have an
     * id */
    const string& getTypeId() const {
        return fFeature->getTypeId();
    }
    
    /* get the id based on feature type, or empty string if it doesn't have an
     * id */
    const string& getHavanaTypeId() const {
        return fFeature->getHavanaTypeId();
    }
    
    /* get the name based on feature type, or empty string if it doesn't have an
     * id */
    const string& getTypeName() const {
        return fFeature->getTypeName();
    }
    
    /* get the biotype based on feature type, or empty string if it doesn't have an
     * id */
    const string& getTypeBiotype() const {
        return fFeature->getTypeBiotype();
    }
    
    /* get exon similarity */
    float getExonSimilarity(const FeatureNode* trans2) const;

    /* get the maximum transcript similarity for a gene */
    float getMaxTranscriptSimilarity(const FeatureNode* gene2,
                                     bool manualOnlyTranscripts=false) const;

    /* is ensembl small non-coding gene */
    bool isAutomaticSmallNonCodingGene() const;

    /* is this an automatic annotation? */
    bool isAutomatic() const {
        return fFeature->fSource == GxfFeature::SOURCE_ENSEMBL;
    }

    /* is this an pseudogene annotation (excluding polymorphic)? */
    bool isPseudogene() const {
        const string& biotype = fFeature->getTypeBiotype();
        return (biotype != "polymorphic_pseudogene")
            and (biotype.find("pseudogene") != biotype.npos);
    }

    bool anyChildWithRemapStatus(unsigned remapStatusSet) const;
    bool allChildWithRemapStatus(unsigned remapStatusSet) const;

    /* recursively get a list features matching the specified filter */
    void getMatching(GxfFeatureVector& hits,
                     function<bool(const GxfFeature*)>(filter)) const {
        if (filter(fFeature)) {
            hits.push_back(fFeature);
        }
        for (int i = 0; i < fChildren.size(); i++) {
            fChildren[i]->getMatching(hits, filter);
        }
    }
    
    
    /* add a child node, linking up parents */
    void addChild(FeatureNode* node) {
        assert(node->fParent == NULL);
        fChildren.push_back(node);
        node->fParent = this;
    }

    /* set remap status to specified value */
    void setRemapStatus(RemapStatus remapStatus) {
        fRemapStatus = remapStatus;
    }

    /* recursively set the remap status. */
    void rsetRemapStatus(RemapStatus remapStatus);

    /* Set the target status. Not recursive */
    void setTargetStatus(TargetStatus targetStatus);

    /* Recursively set the target status. */
    void rsetTargetStatus(TargetStatus targetStatus);

    /* recursively set the target status attribute */
    void rsetTargetStatusAttr();

    /* recursively set the target status attribute node. */
    void rsetSubstitutedMissingTargetAttr(const string& targetVersion);

    /* clone tree */
    FeatureNode* clone() const;

    /* print node for debugging */
    void dumpNode(ostream& fh) const;

    /* recursively print for debugging */
    void dump(ostream& fh) const;

    /* set the remap number of mappings attribute on this node  */
    void setNumMappingsAttr();

    /* recursively set the remap status attribute */
    void rsetRemapStatusAttr();

    /* depth-first output */
    void write(ostream& fh) const;
};

/**
 * Trees resulting from a map.
 */
class ResultFeatureTrees {
    private:
    bool anyChildWithRemapStatus(unsigned remapStatusSet) const;
    bool allChildWithRemapStatus(unsigned remapStatusSet) const;


    public:
    const FeatureNode* src;  /// not owned
    FeatureNode* mapped;
    FeatureNode* unmapped;
    FeatureNode* target;    // substituted from target

    /* constructor */
    ResultFeatureTrees(const FeatureNode* src = NULL,
                       FeatureNode* mapped = NULL,
                       FeatureNode* unmapped = NULL):
        src(src), mapped(mapped), unmapped(unmapped), target(NULL) {
    }

    /* free data all */
    void free() {
        delete mapped;
        delete unmapped;
        delete target;
    }
    /* free mapped */
    void freeMapped() {
        delete mapped;
        mapped = NULL;
    }

    /* free unmapped */
    void freeUnmapped() {
        delete unmapped;
        unmapped = NULL;
    }

    /* get remap status from either mappedm unmapped, or target. */
    RemapStatus getRemapStatus() const {
        if (mapped != NULL) {
            return mapped->fRemapStatus;
        } else if (unmapped != NULL) {
            return unmapped->fRemapStatus;
        } else if (target != NULL) {
            return target->fRemapStatus;
        } else {
            return REMAP_STATUS_DELETED;
        }
    }

    /* recursively set the remap status on mapped and unmapped */
    void rsetRemapStatus(RemapStatus remapStatus) {
        if (mapped != NULL) {
            mapped->rsetRemapStatus(remapStatus);
        }
        if (unmapped != NULL) {
            unmapped->rsetRemapStatus(remapStatus);
        }
    }

    /* get target status from either mapped, unmapped, or target */
    TargetStatus getTargetStatus() const {
        if (mapped != NULL) {
            return mapped->fTargetStatus;
        } else if (unmapped != NULL) {
            return unmapped->fTargetStatus;
        } else if (target != NULL) {
            return target->fTargetStatus;
        } else {
            return TARGET_STATUS_LOST;
        }
    }

    /* set target status on mapped and unmapped trees */
    void setTargetStatus(TargetStatus targetStatus) {
        if (mapped != NULL) {
            mapped->setTargetStatus(targetStatus);
        }
        if (unmapped != NULL) {
            unmapped->setTargetStatus(targetStatus);
        }
    }

    /* recursively set target status on trees */
    void rsetTargetStatus(TargetStatus targetStatus) {
        if (mapped != NULL) {
            mapped->rsetTargetStatus(targetStatus);
        }
        if (unmapped != NULL) {
            unmapped->rsetTargetStatus(targetStatus);
        }
    }

    /* recursively set the target status attribute */
    void rsetTargetStatusAttr() {
        if (mapped != NULL) {
            mapped->rsetTargetStatusAttr();
        }
        if (unmapped != NULL) {
            unmapped->rsetTargetStatusAttr();
        }
    }

    /* get number of mappings from either mapped or unmapped. */
    int getNumMappings() const {
        if (mapped != NULL) {
            return mapped->fNumMappings;
        } else if (unmapped != NULL) {
            return unmapped->fNumMappings;
        } else {
            return 0;
        }
    }

    /* set the remap number of mappings attribute on this node (not
     * recursive) */
    void setNumMappingsAttr() {
        if (mapped != NULL) {
            mapped->setNumMappingsAttr();
        }
        if (unmapped != NULL) {
            unmapped->setNumMappingsAttr();
        }
    }

    /* recursively set the remap status attribute */
    void rsetRemapStatusAttr() {
        if (mapped != NULL) {
            mapped->rsetRemapStatusAttr();
        }
        if (unmapped != NULL) {
            unmapped->rsetRemapStatusAttr();
        }
    }

    /* determine gene or transcript remap status from children.  This doesn't
     * handle GENE_CONFLICT or_GENE_SIZE_CHANGE, which are forced.
     */
    RemapStatus calcBoundingFeatureRemapStatus(bool srcSeqInMapping) const;

    /* set remap status on a bounding */
    void setBoundingFeatureRemapStatus(bool srcSeqInMapping) {
        RemapStatus remapStatus = calcBoundingFeatureRemapStatus(srcSeqInMapping);
        if (mapped != NULL) {
            mapped->setRemapStatus(remapStatus);
        }
        if (unmapped != NULL) {
            unmapped->setRemapStatus(remapStatus);
        }
    }

    /* print for debugging */
    void dump(ostream& fh) const {
        if (mapped != NULL) {
            fh << "@@@ mapped" << endl;
            mapped->dump(fh);
        }
        if (unmapped != NULL) {
            fh << "@@@ unmapped" << endl;
            unmapped->dump(fh);
        }
    }
};

/* vector of mapped resulting feature */
class ResultFeatureTreesVector: public vector<ResultFeatureTrees> {
    public:
    bool haveMapped() const {
        for (int i = 0; i < size(); i++) {
            if (((*this)[i]).mapped != NULL) {
                return true;
            }
        }
        return false;
    }
    bool haveUnmapped() const {
        for (int i = 0; i < size(); i++) {
            if (((*this)[i]).unmapped != NULL) {
                return true;
            }
        }
        return false;
    }
};

/**
 * Set of mapped and unmapped features resulting from transmap.  A feature maybe split when mapped,
 * hence vectors.
 */
class TransMappedFeature {
    public:
    const FeatureNode* src;
    FeatureNodeVector mapped;
    FeatureNodeVector unmapped;

    /* constructors */
    TransMappedFeature(const FeatureNode* src = NULL):
        src(src) {
    }

    TransMappedFeature(ResultFeatureTrees& featureTrees) {
        src = featureTrees.src;
        if (featureTrees.mapped != NULL) {
            mapped.push_back(featureTrees.mapped);
        }
        if (featureTrees.unmapped != NULL) {
            unmapped.push_back(featureTrees.unmapped);
        }
    }

    /** add a mapped node */
    void addMapped(FeatureNode* featureNode) {
        mapped.push_back(featureNode);
    }

    /** add a unmapped node */
    void addUnmapped(FeatureNode* featureNode) {
        unmapped.push_back(featureNode);
    }

    /* compute status for a transmapped feature.  this only looks at a single
     * level of mappings, not a tree. srcSeqInMapping indicates of the srcSequence
     * was in the genomic map */
    RemapStatus calcRemapStatus(bool srcSeqInMapping) const;

    /**
     * calculate the remap status and set it in the feature nodes
     */
    void setRemapStatus(bool srcSeqInMapping) {
        RemapStatus remapStatus = calcRemapStatus(srcSeqInMapping);
        for (int i = 0; i < mapped.size(); i++) {
            mapped[i]->fRemapStatus = remapStatus;
        }
        for (int i = 0; i < unmapped.size(); i++) {
            unmapped[i]->fRemapStatus = remapStatus;
        }
    }
};


/**
 * Group genes records together in a tree.
 */
class GeneTree {
    private:
    static void queueRecords(GxfParser *gxfParser,
                             GxfRecordVector& gxfRecords);
    static FeatureNode* findGff3Parent(FeatureNode* geneTreeLeaf,
                                       const GxfFeature* cfeature);
    static FeatureNode* loadGff3GeneRecord(GxfFeature* feature,
                                           FeatureNode* geneTreeLeaf);
    static const string& getGtfParentType(const string& featureType);
    static FeatureNode* findGtfParent(FeatureNode* geneTreeLeaf,
                                      const GxfFeature* feature);
    static FeatureNode* loadGtfGeneRecord(GxfFeature* feature,
                                          FeatureNode* geneTreeLeaf);    
    static bool loadGeneRecord(GxfParser *gxfParser,
                               GxfRecord* gxfRecord,
                               FeatureNode* geneTreeRoot,
                               FeatureNode*& geneTreeLeaf,
                               GxfRecordVector& queuedRecords);
    static FeatureNode* loadGene(GxfParser *gxfParser,
                                 GxfFeature* geneFeature);
    static void removeTransAttrsOnGenes(FeatureNode* geneTreeRoot);
    static void fixGtfAnnotations(FeatureNode* geneTreeRoot);
    public:
    /* factory */
    static FeatureNode* geneTreeFactory(GxfParser *gxfParser,
                                        GxfFeature* geneFeature);
};

#endif
