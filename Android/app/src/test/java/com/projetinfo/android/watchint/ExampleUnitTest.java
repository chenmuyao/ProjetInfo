package com.projetinfo.android.watchint;

import org.junit.Test;

import static org.junit.Assert.*;

import java.io.ByteArrayOutputStream;

import static org.junit.Assert.assertEquals;

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
public class ExampleUnitTest {

    @Test
    public void testWrite() {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        // pass the baos to be writed with "value", for this example
        byte[] byteArray = baos.toByteArray();
        String str = new String(byteArray);
        assertEquals("value", new String(byteArray));
    }

    public void testStringOf() {

    }



}

