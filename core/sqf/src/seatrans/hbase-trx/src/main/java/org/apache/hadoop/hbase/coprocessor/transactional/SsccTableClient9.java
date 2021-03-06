package org.apache.hadoop.hbase.coprocessor.transactional;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class SsccTableClient9 {
    // ------ for concurrent ------
    protected final Log log = LogFactory.getLog(getClass());
    private static int successNum = 0;

    /**
     * concurrency writes
     * 
     * @param args
     * @throws Exception
     */
    static public void main(String[] args) throws Exception {
        SsccTableClient9 cilent = new SsccTableClient9();
        cilent.concurrencyWrites1();

        System.out.println("=========================================");
        System.out.println(" ");
        System.out.println("TOTAL : 1 . SUCCESS : " + successNum + " FAILURE : " + (1 - successNum));
        System.out.println(" ");
        System.out.println("=========================================");
    }

    /**
     * main--put[v1]--del[v1]--put[v2]--get[v2]<br/>
     */
    private void concurrencyWrites1() {
        try {
            System.out.println("Starting SsccTableClient9: concurrencyWrites1");

            System.out.println("main--put[v1]--del[v1]--put[v2]--get[v2]");

            SsccTableClientUtils.initialize();
            SsccTableClientUtils.testSsccBeginTransaction();

            SsccTableClientUtils.testSsccPut(SsccTableClientUtils.VALUE1);
            SsccTableClientUtils.testSsccDelete(SsccTableClientUtils.VALUE1);
            SsccTableClientUtils.testSsccPut(SsccTableClientUtils.VALUE2);
            SsccTableClientUtils.testSsccCommitIfPossible();
            SsccTableClientUtils.testSsccBeginTransaction();
            Integer[] result = SsccTableClientUtils.testSsccGet();
            SsccTableClientUtils.testSsccCommitIfPossible();
            System.out.println("Finish SsccTableClient9: concurrencyWrites1");

            if (result.length == 1 && result[0] == 2) {
                successNum++;
                System.out.println("=========================================");
                System.out.println(" ");
                System.out.println("SUCCESS");
                System.out.println(" ");
                System.out.println("=========================================");
            } else {
                System.out.println("=========================================");
                System.out.println(" ");
                System.out.println("FAILURE");
                System.out.println(" ");
                System.out.println("=========================================");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}