typedef union pt_eptp {
	u64 All;
    struct {
        u64 MemoryType : 3; // bit 2:0 (0 = Uncacheable (UC) - 6 = Write - back(WB))
        u64 PageWalkLength : 3; // bit 5:3 (This value is 1 less than the EPT page-walk length) 
        u64 DirtyAndAceessEnabled : 1; // bit 6  (Setting this control to 1 enables accessed and dirty flags for EPT)
        u64 Reserved1 : 5; // bit 11:7 
        u64 PML4Address : 36;
        u64 Reserved2 : 16;
    } fields;
} eptp, *peptp;

typedef union  pt_pml4e{
    u64 All;
    struct {
        u64 Read : 1; // bit 0
        u64 Write : 1; // bit 1
        u64 Execute : 1; // bit 2
        u64 Reserved1 : 5; // bit 7:3 (Must be Zero)
        u64 Accessed : 1; // bit 8
        u64 Ignored1 : 1; // bit 9
        u64 ExecuteForUserMode : 1; // bit 10
        u64 Ignored2 : 1; // bit 11
        u64 PhysicalAddress : 36; // bit (N-1):12 or Page-Frame-Number
        u64 Reserved2 : 4; // bit 51:N
        u64 Ignored3 : 12; // bit 63:52
    }fields;
} ept_pml4e, *pept_pml4e;

typedef union pt_pdpte {
    u64 All;
    struct {
        u64 Read : 1; // bit 0
        u64 Write : 1; // bit 1
        u64 Execute : 1; // bit 2
        u64 Reserved1 : 5; // bit 7:3 (Must be Zero)
        u64 Accessed : 1; // bit 8
        u64 Ignored1 : 1; // bit 9
        u64 ExecuteForUserMode : 1; // bit 10
        u64 Ignored2 : 1; // bit 11
        u64 PhysicalAddress : 36; // bit (N-1):12 or Page-Frame-Number
        u64 Reserved2 : 4; // bit 51:N
        u64 Ignored3 : 12; // bit 63:52
    } fields;
}ept_pdpte, *pept_pdpte;

typedef union pt_pde {
    u64 All;
    struct {
        u64 Read : 1; // bit 0
        u64 Write : 1; // bit 1
        u64 Execute : 1; // bit 2
        u64 Reserved1 : 5; // bit 7:3 (Must be Zero)
        u64 Accessed : 1; // bit 8
        u64 Ignored1 : 1; // bit 9
        u64 ExecuteForUserMode : 1; // bit 10
        u64 Ignored2 : 1; // bit 11
        u64 PhysicalAddress : 36; // bit (N-1):12 or Page-Frame-Number
        u64 Reserved2 : 4; // bit 51:N
        u64 Ignored3 : 12; // bit 63:52
    } fields;
} ept_pde, *pept_pde;

typedef union pt_pte {
    u64 All;
    struct {
        u64 Read : 1; // bit 0
        u64 Write : 1; // bit 1
        u64 Execute : 1; // bit 2
        u64 EPTMemoryType : 3; // bit 5:3 (EPT Memory type)
        u64 IgnorePAT : 1; // bit 6
        u64 Ignored1 : 1; // bit 7
        u64 AccessedFlag : 1; // bit 8   
        u64 DirtyFlag : 1; // bit 9
        u64 ExecuteForUserMode : 1; // bit 10
        u64 Ignored2 : 1; // bit 11
        u64 PhysicalAddress : 36; // bit (N-1):12 or Page-Frame-Number
        u64 Reserved : 4; // bit 51:N
        u64 Ignored3 : 11; // bit 62:52
        u64 SuppressVE : 1; // bit 63
    } fields;
} ept_pte, *pept_pte;
