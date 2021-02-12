/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.fibase.mediator;

import java.io.FileInputStream;
import java.io.InputStream;
import org.apache.jena.ontology.OntClass;
import org.apache.jena.ontology.OntModel;
import org.apache.jena.ontology.OntModelSpec;
import org.apache.jena.ontology.OntProperty;
import org.apache.jena.rdf.model.ModelFactory;
import org.apache.jena.util.iterator.ExtendedIterator;

/**
 *
 * @author andre
 */
public class Ontology {

    private final OntModel model;
    private final String namespace;

    public Ontology(String Path) {
        this.model = ModelFactory.createOntologyModel(OntModelSpec.OWL_DL_MEM);
        try {
            InputStream in = new FileInputStream(Path);
            if (in == null) {
                System.out.println("File not found");
            }
            model.read(in, " ");

        } catch (Exception e) {
            System.out.println("model.read catched error: " + e);
        }

        this.namespace = model.getNsPrefixURI("or_obsnertwork");
    }

    public void printAllEquivalents() {
        OntClass obj = model.getOntClass(namespace + "Data");
        ExtendedIterator<OntProperty> propIter = obj.listDeclaredProperties(false);
        if (propIter.hasNext()) {
            while (propIter.hasNext()) {

                final OntProperty ontProperty = (OntProperty) propIter.next();
                ExtendedIterator<? extends OntProperty> eqProp = ontProperty.listEquivalentProperties();
                if (eqProp.hasNext()) {
                    System.out.print(ontProperty.getLocalName() + " é equivalente ----->> ");
                    while (eqProp.hasNext()) {
                        OntProperty property = (OntProperty) eqProp.next();
                        System.out.println(property.getLocalName());
                    }
                }

            }
        }
    }

    public String getEquivalent(String data) {
        OntProperty ontProperty = model.getOntProperty(namespace + data);
        if (ontProperty != null) {
            ExtendedIterator<? extends OntProperty> eqProp = ontProperty.listEquivalentProperties();
            if (eqProp.hasNext()) {
                while (eqProp.hasNext()) {
                    OntProperty property = (OntProperty) eqProp.next();
                    return property.getLocalName();
                }
            }

            return getReverseEquivalent(data);
        } else {
            return "";
        }

    }

    public String getReverseEquivalent(String data) {

        OntClass obj = model.getOntClass(namespace + "Data");
        ExtendedIterator<OntProperty> propIter = obj.listDeclaredProperties(false);
        if (propIter.hasNext()) {
            while (propIter.hasNext()) {

                final OntProperty ontProperty = (OntProperty) propIter.next();
                ExtendedIterator<? extends OntProperty> eqProp = ontProperty.listEquivalentProperties();

                if (eqProp.hasNext()) {
                    while (eqProp.hasNext()) {
                        OntProperty property = (OntProperty) eqProp.next();
                        if (property.getLocalName().equals(data)) {
                            return ontProperty.getLocalName();
                        }
                    }
                }

            }
        }
        return "";

    }

}
