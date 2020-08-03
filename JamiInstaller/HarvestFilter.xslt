<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:wix="http://schemas.microsoft.com/wix/2006/wi">

    <xsl:output method="xml" indent="yes" />

    <xsl:template match="@*|node()">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
    </xsl:template>

    <xsl:key name="service-search" match="wix:Component[contains(wix:File/@Source, 'Jami.exe')]" use="@Id" />
    <xsl:key name="vc-service-search" match="wix:Component[contains(wix:File/@Source, 'vcredist_x64.exe')]" use="@Id" />
    <xsl:key name="pdb-search" match="wix:Component[contains(wix:File/@Source, '.pdb')]" use="@Id" />
    <xsl:key name="lib-search" match="wix:Component[contains(wix:File/@Source, 'Jami.lib')]" use="@Id" />
    <xsl:key name="exp-search" match="wix:Component[contains(wix:File/@Source, 'Jami.exp')]" use="@Id" />
    <xsl:key name="qmake-search" match="wix:Component[contains(wix:File/@Source, 'qmake')]" use="@Id" />
    <xsl:key name="obj-search" match="wix:Component[contains(wix:File/@Source, '.obj')]" use="@Id" />
    <xsl:key name="tlog-search" match="wix:Component[contains(wix:File/@Source, '.tlog')]" use="@Id" />
    <xsl:key name="log-search" match="wix:Component[contains(wix:File/@Source, '.log')]" use="@Id" />

    <xsl:template match="wix:Component[key('service-search', @Id)]" />
    <xsl:template match="wix:Component[key('vc-service-search', @Id)]" />
    <xsl:template match="wix:Component[key('pdb-search', @Id)]" />
    <xsl:template match="wix:Component[key('lib-search', @Id)]" />
    <xsl:template match="wix:Component[key('exp-search', @Id)]" />
    <xsl:template match="wix:Component[key('qmake-search', @Id)]" />
    <xsl:template match="wix:Component[key('obj-search', @Id)]" />
    <xsl:template match="wix:Component[key('tlog-search', @Id)]" />
    <xsl:template match="wix:Component[key('log-search', @Id)]" />

    <xsl:template match="wix:ComponentRef[key('service-search', @Id)]" />
    <xsl:template match="wix:ComponentRef[key('vc-service-search', @Id)]" />
    <xsl:template match="wix:ComponentRef[key('pdb-search', @Id)]" />
    <xsl:template match="wix:ComponentRef[key('lib-search', @Id)]" />
    <xsl:template match="wix:ComponentRef[key('exp-search', @Id)]" />
    <xsl:template match="wix:ComponentRef[key('qmake-search', @Id)]" />
    <xsl:template match="wix:ComponentRef[key('obj-search', @Id)]" />
    <xsl:template match="wix:ComponentRef[key('tlog-search', @Id)]" />
    <xsl:template match="wix:ComponentRef[key('log-search', @Id)]" />

</xsl:stylesheet>