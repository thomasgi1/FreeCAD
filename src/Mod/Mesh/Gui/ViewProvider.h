/***************************************************************************
 *   Copyright (c) 2004 Werner Mayer <wmayer[at]users.sourceforge.net>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef MESHGUI_VIEWPROVIDERMESH_H
#define MESHGUI_VIEWPROVIDERMESH_H

#include <vector>
#include <Inventor/fields/SoSFVec2f.h>

#include <Mod/Mesh/App/Core/Elements.h>
#include <Mod/Mesh/App/Types.h>
#include <Gui/ViewProviderGeometryObject.h>
#include <Gui/ViewProviderBuilder.h>
#include <App/PropertyStandard.h>


class SoGroup;
class SoSeparator;
class SoEventCallback;
class SbViewVolume;
class SoBaseColor;
class SoShape;
class SoCoordinate3;
class SoIndexedFaceSet;
class SoShapeHints;
class SoMaterialBinding;
class SoCamera;
class SoAction;
class SbViewportRegion;
class SbVec2f;
class SbBox2s;
class SbPlane;

namespace App {
  class Color;
  class PropertyColorList;
}

namespace Base {
  class ViewProjMethod;
}

namespace Gui {
  class View3DInventorViewer;
  class SoFCSelection;
}


namespace MeshCore {
  class MeshKernel;
  struct Material;
}


namespace MeshGui {
class SoFCMeshObjectNode;
class SoFCMeshObjectShape;

class MeshGuiExport ViewProviderMeshBuilder : public Gui::ViewProviderBuilder
{
public:
    ViewProviderMeshBuilder(){}
    ~ViewProviderMeshBuilder() override{}
    void buildNodes(const App::Property*, std::vector<SoNode*>&) const override;
    void createMesh(const App::Property*, SoCoordinate3*, SoIndexedFaceSet*) const;
    void createMesh(const MeshCore::MeshKernel&, SoCoordinate3*, SoIndexedFaceSet*) const;
};

/**
 * The ViewProviderExport class creates an empty node.
 * @author Werner Mayer
 */
class MeshGuiExport ViewProviderExport : public Gui::ViewProviderDocumentObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(MeshGui::ViewProviderExport);

public:
    ViewProviderExport();
    ~ViewProviderExport() override;

    QIcon getIcon() const override;
    SoSeparator* getRoot() const override {return nullptr;}
    std::vector<std::string> getDisplayModes() const override;
    const char* getDefaultDisplayMode() const override;
};

/**
 * The ViewProviderMesh class offers the visualization of the mesh data structure
 * and many algorithms to work on or edit the mesh.
 * @author Werner Mayer
 */
class MeshGuiExport ViewProviderMesh : public Gui::ViewProviderGeometryObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(MeshGui::ViewProviderMesh);

public:
    ViewProviderMesh();
    ~ViewProviderMesh() override;

    // Display properties
    App::PropertyPercent LineTransparency;
    App::PropertyFloatConstraint LineWidth;
    App::PropertyFloatConstraint PointSize;
    App::PropertyFloatConstraint CreaseAngle;
    App::PropertyBool OpenEdges;
    App::PropertyBool Coloring;
    App::PropertyEnumeration Lighting;
    App::PropertyColor LineColor;

    void attach(App::DocumentObject *) override;
    void updateData(const App::Property*) override;
    bool useNewSelectionModel() const override {return false;}
    Gui::SoFCSelection* getHighlightNode() const { return pcHighlight; }
    QIcon getIcon() const override;
    /// Sets the correct display mode
    void setDisplayMode(const char* ModeName) override;
    /// returns a list of all possible modes
    std::vector<std::string> getDisplayModes() const override;
    bool exportToVrml(const char* filename, const MeshCore::Material&, bool binary=false) const;
    void exportMesh(const char* filename, const char* fmt=nullptr) const;
    void setupContextMenu(QMenu*, QObject*, const char*) override;
    /// Get the python wrapper for that ViewProvider
    PyObject* getPyObject() override;

    /** @name Editing */
    //@{
    bool doubleClicked() override{ return false; }
    bool isFacetSelected(Mesh::FacetIndex facet);
    void selectComponent(Mesh::FacetIndex facet);
    void deselectComponent(Mesh::FacetIndex facet);
    void selectFacet(Mesh::FacetIndex facet);
    void deselectFacet(Mesh::FacetIndex facet);
    void setSelection(const std::vector<Mesh::FacetIndex>&);
    void addSelection(const std::vector<Mesh::FacetIndex>&);
    void removeSelection(const std::vector<Mesh::FacetIndex>&);
    void invertSelection();
    void clearSelection();
    void deleteSelection();
    bool hasSelection() const;
    void getFacetsFromPolygon(const std::vector<SbVec2f>& picked,
                              const Base::ViewProjMethod& proj, SbBool inner,
                              std::vector<Mesh::FacetIndex>& indices) const;
    std::vector<Mesh::FacetIndex> getFacetsOfRegion(const SbViewportRegion&, const SbViewportRegion&, SoCamera*) const;
    std::vector<Mesh::FacetIndex> getVisibleFacetsAfterZoom(const SbBox2s&, const SbViewportRegion&, SoCamera*) const;
    std::vector<Mesh::FacetIndex> getVisibleFacets(const SbViewportRegion&, SoCamera*) const;
    virtual void cutMesh(const std::vector<SbVec2f>& picked, const Base::ViewProjMethod& proj, SbBool inner);
    virtual void trimMesh(const std::vector<SbVec2f>& picked, const Base::ViewProjMethod& proj, SbBool inner);
    virtual void appendFacets(const std::vector<Mesh::FacetIndex>&);
    virtual void removeFacets(const std::vector<Mesh::FacetIndex>&);
    /*! The size of the array must be equal to the number of facets. */
    void setFacetTransparency(const std::vector<float>&);
    void resetFacetTransparency();
    void highlightSegments(const std::vector<App::Color>&);
    //@}

protected:
    /// Sets the edit mode
    bool setEdit(int ModNum) override;
    /// Unsets the edit mode
    void unsetEdit(int ModNum) override;
    /// get called by the container whenever a property has been changed
    void onChanged(const App::Property* prop) override;
    virtual void showOpenEdges(bool);
    void setOpenEdgeColorFrom(const App::Color& col);
    virtual void splitMesh(const MeshCore::MeshKernel& toolMesh, const Base::Vector3f& normal, SbBool inner);
    virtual void segmentMesh(const MeshCore::MeshKernel& toolMesh, const Base::Vector3f& normal, SbBool inner);
    virtual void faceInfo(Mesh::FacetIndex facet);
    virtual void fillHole(Mesh::FacetIndex facet);
    virtual void selectArea(short, short, short, short, const SbViewportRegion&, SoCamera*);
    virtual void highlightSelection();
    virtual void unhighlightSelection();
    void highlightComponents();
    void setHighlightedComponents(bool);
    void highlightSegments();
    void setHighlightedSegments(bool);
    void setHighlightedColors(bool);
    void highlightColors();
    bool canHighlightColors() const;
    App::PropertyColorList* getColorProperty() const;
    App::PropertyFloatList* getTransparencyProperty() const;
    void tryColorPerVertexOrFace(bool);
    void setColorPerVertex(const App::PropertyColorList*);
    void setColorPerFace(const App::PropertyColorList*);
    void tryTransparency(bool);

    virtual SoShape* getShapeNode() const;
    virtual SoNode* getCoordNode() const;

public:
    static void faceInfoCallback(void * ud, SoEventCallback * n);
    static void fillHoleCallback(void * ud, SoEventCallback * n);
    static void markPartCallback(void * ud, SoEventCallback * n);
    static void clipMeshCallback(void * ud, SoEventCallback * n);
    static void trimMeshCallback(void * ud, SoEventCallback * n);
    static void partMeshCallback(void * ud, SoEventCallback * n);
    static void segmMeshCallback(void * ud, SoEventCallback * n);
    static void selectGLCallback(void * ud, SoEventCallback * n);
    /// Creates a tool mesh from the previous picked polygon on the viewer
    static bool createToolMesh(const std::vector<SbVec2f>& rclPoly, const SbViewVolume& vol,
            const Base::Vector3f& rcNormal, std::vector<MeshCore::MeshGeomFacet>&);

private:
    static void renderGLCallback(void * ud, SoAction * a);
    static void boxZoom(const SbBox2s& box, const SbViewportRegion & vp, SoCamera* cam);
    static void panCamera(SoCamera*, float, const SbPlane&, const SbVec2f&, const SbVec2f&);

protected:
    enum class HighlighMode {
        None,
        Component,
        Segment,
        Color
    };
    HighlighMode highlightMode;
    Gui::SoFCSelection  * pcHighlight;
    SoGroup             * pcShapeGroup;
    SoDrawStyle         * pcLineStyle;
    SoDrawStyle         * pcPointStyle;
    SoSeparator         * pcOpenEdge;
    SoBaseColor         * pOpenColor;
    SoMaterial          * pLineColor;
    SoShapeHints        * pShapeHints;
    SoMaterialBinding   * pcMatBinding;

private:
    static App::PropertyFloatConstraint::Constraints floatRange;
    static App::PropertyFloatConstraint::Constraints angleRange;
    static App::PropertyIntegerConstraint::Constraints intPercent;
    static const char* LightingEnums[];
};

/**
 * The ViewProviderIndexedFaceSet class creates an indexed faceset node in order
 * to render the mesh data structure.
 * @author Werner Mayer
 */
class MeshGuiExport ViewProviderIndexedFaceSet : public ViewProviderMesh
{
    PROPERTY_HEADER_WITH_OVERRIDE(MeshGui::ViewProviderIndexedFaceSet);

public:
    ViewProviderIndexedFaceSet();
    ~ViewProviderIndexedFaceSet() override;

    void attach(App::DocumentObject *) override;
    /// Update the Mesh representation
    void updateData(const App::Property*) override;

protected:
    void showOpenEdges(bool) override;
    SoShape* getShapeNode() const override;
    SoNode* getCoordNode() const override;

private:
    SoCoordinate3       * pcMeshCoord;
    SoIndexedFaceSet    * pcMeshFaces;
};

/**
 * The ViewProviderIndexedFaceSet class creates an own node in order
 * to directly render the mesh data structure.
 * @author Werner Mayer
 */
class MeshGuiExport ViewProviderMeshObject : public ViewProviderMesh
{
    PROPERTY_HEADER_WITH_OVERRIDE(MeshGui::ViewProviderMeshObject);

public:
    ViewProviderMeshObject();
    ~ViewProviderMeshObject() override;

    void attach(App::DocumentObject *pcFeat) override;
    void updateData(const App::Property*) override;

protected:
    SoShape* getShapeNode() const override;
    SoNode* getCoordNode() const override;
    void showOpenEdges(bool) override;

private:
    SoFCMeshObjectNode  * pcMeshNode;
    SoFCMeshObjectShape * pcMeshShape;
};

} // namespace MeshGui


#endif // MESHGUI_VIEWPROVIDERMESH_H

