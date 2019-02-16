/*
 * map of location of target gene and transcripts. used in selecting
 * from multiple mappings
 */
#ifndef annotationSet_hh
#define annotationSet_hh

#include "gxf.hh"
#include <map>
#include <stdexcept>
#include "featureTree.hh"
struct genomeRangeTree;
class GenomeSizeMap;
class GxfWriter;

/*
 * Locations in target genome of old transcripts, by base id
 */
class AnnotationSet {
    private:
    /* stored in range tree to link target location to
     * tree. */
    struct LocationLink {
        struct LocationLink *next;
        FeatureNode* feature;
    };
 
       
    // map of gene or transcripts to features. Keep up to two for PAR
    typedef map<const string, FeatureNodeVector> FeatureMap;
    typedef FeatureMap::iterator FeatureMapIter;
    typedef FeatureMap::const_iterator FeatureMapConstIter;

    // map by base id of genes and transcripts (not exons).
    FeatureMap fIdFeatureMap;

    // map by names of genes and transcripts
    FeatureMap fNameFeatureMap;

    // list of all gene features found
    FeatureNodeVector fGenes;

    // map of location to feature
    struct genomeRangeTree* fLocationMap;

    // mapped sequence ids that have been written
    StringSet fSeqRegionsWritten;

    // optional table of chromosome sequence sizes
    const GenomeSizeMap* fGenomeSizes;
    
    void addFeature(FeatureNode* feature);
    void processRecord(GxfParser *gxfParser,
                       GxfRecord* gxfRecord);
    void addLocationMap(FeatureNode* feature);
    void buildLocationMap();
    void freeLocationMap();
    bool isOverlappingGene(const FeatureNode* gene,
                           const FeatureNode* overlappingFeature,
                           float minSimilarity,
                           bool manualOnlyTranscripts);

    FeatureNode* getFeatureByKey(const string& key,
                                 const FeatureMap& featureMap,
                                 const string& seqIdForParCheck) const;

    /* check if a seqregion for seqid has been written, if so, return true,
     * otherwise record it and return false.  */
    bool checkRecordSeqRegionWritten(const string& seqid) {
        if (fSeqRegionsWritten.find(seqid) == fSeqRegionsWritten.end()) {
            fSeqRegionsWritten.insert(seqid);
            return false;
        } else {
            return true;
        }
    }
    void outputSeqRegion(const string& seqId,
                         int size,
                         GxfWriter& gxfFh);
    void outputMappedSeqRegionIfNeed(const FeatureNode* gene,
                                     GxfWriter& mappedGxfFh);
    void outputFeature(const FeatureNode* feature,
                       GxfWriter& gxfFh) const;

    public:
    /* constructor, load gene and transcript objects from a GxF */
    AnnotationSet(const string& gxfFile,
                  const GenomeSizeMap* genomeSizes=NULL);

    /* constructor, empty set */
    AnnotationSet(const GenomeSizeMap* genomeSizes=NULL):
        fLocationMap(NULL),
        fGenomeSizes(genomeSizes) {
    }

    /* destructor */
    ~AnnotationSet();

    /* add a gene the maps */
    void addGene(FeatureNode* gene);

    /* get a gene or transcript with same base id or NULL.  special
     * handling for PARs. */
    FeatureNode* getFeatureById(const string& id,
                            const string& seqIdForParCheck) const;

    /* get a gene or transcript with same name or NULL.  special handling
     * for PARs. */
    FeatureNode* getFeatureByName(const string& name,
                              const string& seqIdForParCheck) const;

    /* find overlapping features */
    FeatureNodeVector findOverlappingFeatures(const string& seqid,
                                          int start,
                                          int end);
    
    /* find overlapping genes with minimum similarity at the transcript level */
    FeatureNodeVector findOverlappingGenes(const FeatureNode* gene,
                                       float minSimilarity,
                                       bool manualOnlyTranscripts);

    /* get list of all gene features */
    const FeatureNodeVector& getGenes() const {
        return fGenes;
    }

    /* sort genes */
    void sort() {
        fGenes.sort();
    }
    
    /* print for debugging */
    void dump(ostream& fh) const;

    /* output genes */
    void write(GxfWriter& gxfFh);
};

#endif
