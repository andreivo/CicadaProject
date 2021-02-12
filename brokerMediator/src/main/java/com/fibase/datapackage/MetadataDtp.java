/*
 * *****************************************************************************
 *  Ministério da Ciência, Tecnologia, Inovações e Comunicações (MCTIC)
 *  Centro Nacional de Monitoramento e Alertas de Desastres Naturais (CEMADEN)
 *
 *  Este documento contém informações confidenciais e está protegido pela legis-
 *  lação brasileira (Constituição Federal, Art. 5º, inciso XII; Lei 9.296/1996,
 *  Art. 10; Lei 12.965/2014 Art. 7º, incisos I, II e III). Seu conteúdo é diri-
 *  gido ao uso do indivíduo ou da entidade mencionados acima.
 *
 *  This document contains confidential information protected by Brazilian laws
 *  and intended only for the use of the individual or entity named above.
 *
 * *****************************************************************************
 */
package com.fibase.datapackage;

import java.time.LocalDateTime;

public class MetadataDtp {

    private String dataType;
    private LocalDateTime collectDateTime;
    private String dataValue;
    private String context;

    public MetadataDtp() {
    }

    public MetadataDtp(String dataType, LocalDateTime collectDateTime, String dataValue) {
        this.dataType = dataType;
        this.collectDateTime = collectDateTime;
        this.dataValue = dataValue;
    }

    public String getDataType() {
        return dataType;
    }

    public void setDataType(String dataType) {
        this.dataType = dataType;
    }

    public LocalDateTime getCollectDateTime() {
        return collectDateTime;
    }

    public void setCollectDateTime(LocalDateTime collectDateTime) {
        this.collectDateTime = collectDateTime;
    }

    public String getDataValue() {
        return dataValue;
    }

    public void setDataValue(String dataValue) {
        this.dataValue = dataValue;
    }

    public String getContext() {
        return context;
    }

    public void setContext(String context) {
        this.context = context;
    }

}
