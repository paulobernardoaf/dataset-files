

const uint64_t lzma_crc64_table[4][256] = {
{
UINT64_C(0x0000000000000000), UINT64_C(0x6F5FA703BE4C2EB3),
UINT64_C(0x5BA040A8573684F4), UINT64_C(0x34FFE7ABE97AAA47),
UINT64_C(0x335E8FFF84C3D07B), UINT64_C(0x5C0128FC3A8FFEC8),
UINT64_C(0x68FECF57D3F5548F), UINT64_C(0x07A168546DB97A3C),
UINT64_C(0x66BC1EFF0987A1F7), UINT64_C(0x09E3B9FCB7CB8F44),
UINT64_C(0x3D1C5E575EB12503), UINT64_C(0x5243F954E0FD0BB0),
UINT64_C(0x55E291008D44718C), UINT64_C(0x3ABD360333085F3F),
UINT64_C(0x0E42D1A8DA72F578), UINT64_C(0x611D76AB643EDBCB),
UINT64_C(0x4966335138A19B7D), UINT64_C(0x2639945286EDB5CE),
UINT64_C(0x12C673F96F971F89), UINT64_C(0x7D99D4FAD1DB313A),
UINT64_C(0x7A38BCAEBC624B06), UINT64_C(0x15671BAD022E65B5),
UINT64_C(0x2198FC06EB54CFF2), UINT64_C(0x4EC75B055518E141),
UINT64_C(0x2FDA2DAE31263A8A), UINT64_C(0x40858AAD8F6A1439),
UINT64_C(0x747A6D066610BE7E), UINT64_C(0x1B25CA05D85C90CD),
UINT64_C(0x1C84A251B5E5EAF1), UINT64_C(0x73DB05520BA9C442),
UINT64_C(0x4724E2F9E2D36E05), UINT64_C(0x287B45FA5C9F40B6),
UINT64_C(0x92CC66A2704237FB), UINT64_C(0xFD93C1A1CE0E1948),
UINT64_C(0xC96C260A2774B30F), UINT64_C(0xA633810999389DBC),
UINT64_C(0xA192E95DF481E780), UINT64_C(0xCECD4E5E4ACDC933),
UINT64_C(0xFA32A9F5A3B76374), UINT64_C(0x956D0EF61DFB4DC7),
UINT64_C(0xF470785D79C5960C), UINT64_C(0x9B2FDF5EC789B8BF),
UINT64_C(0xAFD038F52EF312F8), UINT64_C(0xC08F9FF690BF3C4B),
UINT64_C(0xC72EF7A2FD064677), UINT64_C(0xA87150A1434A68C4),
UINT64_C(0x9C8EB70AAA30C283), UINT64_C(0xF3D11009147CEC30),
UINT64_C(0xDBAA55F348E3AC86), UINT64_C(0xB4F5F2F0F6AF8235),
UINT64_C(0x800A155B1FD52872), UINT64_C(0xEF55B258A19906C1),
UINT64_C(0xE8F4DA0CCC207CFD), UINT64_C(0x87AB7D0F726C524E),
UINT64_C(0xB3549AA49B16F809), UINT64_C(0xDC0B3DA7255AD6BA),
UINT64_C(0xBD164B0C41640D71), UINT64_C(0xD249EC0FFF2823C2),
UINT64_C(0xE6B60BA416528985), UINT64_C(0x89E9ACA7A81EA736),
UINT64_C(0x8E48C4F3C5A7DD0A), UINT64_C(0xE11763F07BEBF3B9),
UINT64_C(0xD5E8845B929159FE), UINT64_C(0xBAB723582CDD774D),
UINT64_C(0xA187C3EBCA2BB664), UINT64_C(0xCED864E8746798D7),
UINT64_C(0xFA2783439D1D3290), UINT64_C(0x9578244023511C23),
UINT64_C(0x92D94C144EE8661F), UINT64_C(0xFD86EB17F0A448AC),
UINT64_C(0xC9790CBC19DEE2EB), UINT64_C(0xA626ABBFA792CC58),
UINT64_C(0xC73BDD14C3AC1793), UINT64_C(0xA8647A177DE03920),
UINT64_C(0x9C9B9DBC949A9367), UINT64_C(0xF3C43ABF2AD6BDD4),
UINT64_C(0xF46552EB476FC7E8), UINT64_C(0x9B3AF5E8F923E95B),
UINT64_C(0xAFC512431059431C), UINT64_C(0xC09AB540AE156DAF),
UINT64_C(0xE8E1F0BAF28A2D19), UINT64_C(0x87BE57B94CC603AA),
UINT64_C(0xB341B012A5BCA9ED), UINT64_C(0xDC1E17111BF0875E),
UINT64_C(0xDBBF7F457649FD62), UINT64_C(0xB4E0D846C805D3D1),
UINT64_C(0x801F3FED217F7996), UINT64_C(0xEF4098EE9F335725),
UINT64_C(0x8E5DEE45FB0D8CEE), UINT64_C(0xE10249464541A25D),
UINT64_C(0xD5FDAEEDAC3B081A), UINT64_C(0xBAA209EE127726A9),
UINT64_C(0xBD0361BA7FCE5C95), UINT64_C(0xD25CC6B9C1827226),
UINT64_C(0xE6A3211228F8D861), UINT64_C(0x89FC861196B4F6D2),
UINT64_C(0x334BA549BA69819F), UINT64_C(0x5C14024A0425AF2C),
UINT64_C(0x68EBE5E1ED5F056B), UINT64_C(0x07B442E253132BD8),
UINT64_C(0x00152AB63EAA51E4), UINT64_C(0x6F4A8DB580E67F57),
UINT64_C(0x5BB56A1E699CD510), UINT64_C(0x34EACD1DD7D0FBA3),
UINT64_C(0x55F7BBB6B3EE2068), UINT64_C(0x3AA81CB50DA20EDB),
UINT64_C(0x0E57FB1EE4D8A49C), UINT64_C(0x61085C1D5A948A2F),
UINT64_C(0x66A93449372DF013), UINT64_C(0x09F6934A8961DEA0),
UINT64_C(0x3D0974E1601B74E7), UINT64_C(0x5256D3E2DE575A54),
UINT64_C(0x7A2D961882C81AE2), UINT64_C(0x1572311B3C843451),
UINT64_C(0x218DD6B0D5FE9E16), UINT64_C(0x4ED271B36BB2B0A5),
UINT64_C(0x497319E7060BCA99), UINT64_C(0x262CBEE4B847E42A),
UINT64_C(0x12D3594F513D4E6D), UINT64_C(0x7D8CFE4CEF7160DE),
UINT64_C(0x1C9188E78B4FBB15), UINT64_C(0x73CE2FE4350395A6),
UINT64_C(0x4731C84FDC793FE1), UINT64_C(0x286E6F4C62351152),
UINT64_C(0x2FCF07180F8C6B6E), UINT64_C(0x4090A01BB1C045DD),
UINT64_C(0x746F47B058BAEF9A), UINT64_C(0x1B30E0B3E6F6C129),
UINT64_C(0x420F87D795576CC9), UINT64_C(0x2D5020D42B1B427A),
UINT64_C(0x19AFC77FC261E83D), UINT64_C(0x76F0607C7C2DC68E),
UINT64_C(0x715108281194BCB2), UINT64_C(0x1E0EAF2BAFD89201),
UINT64_C(0x2AF1488046A23846), UINT64_C(0x45AEEF83F8EE16F5),
UINT64_C(0x24B399289CD0CD3E), UINT64_C(0x4BEC3E2B229CE38D),
UINT64_C(0x7F13D980CBE649CA), UINT64_C(0x104C7E8375AA6779),
UINT64_C(0x17ED16D718131D45), UINT64_C(0x78B2B1D4A65F33F6),
UINT64_C(0x4C4D567F4F2599B1), UINT64_C(0x2312F17CF169B702),
UINT64_C(0x0B69B486ADF6F7B4), UINT64_C(0x6436138513BAD907),
UINT64_C(0x50C9F42EFAC07340), UINT64_C(0x3F96532D448C5DF3),
UINT64_C(0x38373B79293527CF), UINT64_C(0x57689C7A9779097C),
UINT64_C(0x63977BD17E03A33B), UINT64_C(0x0CC8DCD2C04F8D88),
UINT64_C(0x6DD5AA79A4715643), UINT64_C(0x028A0D7A1A3D78F0),
UINT64_C(0x3675EAD1F347D2B7), UINT64_C(0x592A4DD24D0BFC04),
UINT64_C(0x5E8B258620B28638), UINT64_C(0x31D482859EFEA88B),
UINT64_C(0x052B652E778402CC), UINT64_C(0x6A74C22DC9C82C7F),
UINT64_C(0xD0C3E175E5155B32), UINT64_C(0xBF9C46765B597581),
UINT64_C(0x8B63A1DDB223DFC6), UINT64_C(0xE43C06DE0C6FF175),
UINT64_C(0xE39D6E8A61D68B49), UINT64_C(0x8CC2C989DF9AA5FA),
UINT64_C(0xB83D2E2236E00FBD), UINT64_C(0xD762892188AC210E),
UINT64_C(0xB67FFF8AEC92FAC5), UINT64_C(0xD920588952DED476),
UINT64_C(0xEDDFBF22BBA47E31), UINT64_C(0x8280182105E85082),
UINT64_C(0x8521707568512ABE), UINT64_C(0xEA7ED776D61D040D),
UINT64_C(0xDE8130DD3F67AE4A), UINT64_C(0xB1DE97DE812B80F9),
UINT64_C(0x99A5D224DDB4C04F), UINT64_C(0xF6FA752763F8EEFC),
UINT64_C(0xC205928C8A8244BB), UINT64_C(0xAD5A358F34CE6A08),
UINT64_C(0xAAFB5DDB59771034), UINT64_C(0xC5A4FAD8E73B3E87),
UINT64_C(0xF15B1D730E4194C0), UINT64_C(0x9E04BA70B00DBA73),
UINT64_C(0xFF19CCDBD43361B8), UINT64_C(0x90466BD86A7F4F0B),
UINT64_C(0xA4B98C738305E54C), UINT64_C(0xCBE62B703D49CBFF),
UINT64_C(0xCC47432450F0B1C3), UINT64_C(0xA318E427EEBC9F70),
UINT64_C(0x97E7038C07C63537), UINT64_C(0xF8B8A48FB98A1B84),
UINT64_C(0xE388443C5F7CDAAD), UINT64_C(0x8CD7E33FE130F41E),
UINT64_C(0xB8280494084A5E59), UINT64_C(0xD777A397B60670EA),
UINT64_C(0xD0D6CBC3DBBF0AD6), UINT64_C(0xBF896CC065F32465),
UINT64_C(0x8B768B6B8C898E22), UINT64_C(0xE4292C6832C5A091),
UINT64_C(0x85345AC356FB7B5A), UINT64_C(0xEA6BFDC0E8B755E9),
UINT64_C(0xDE941A6B01CDFFAE), UINT64_C(0xB1CBBD68BF81D11D),
UINT64_C(0xB66AD53CD238AB21), UINT64_C(0xD935723F6C748592),
UINT64_C(0xEDCA9594850E2FD5), UINT64_C(0x829532973B420166),
UINT64_C(0xAAEE776D67DD41D0), UINT64_C(0xC5B1D06ED9916F63),
UINT64_C(0xF14E37C530EBC524), UINT64_C(0x9E1190C68EA7EB97),
UINT64_C(0x99B0F892E31E91AB), UINT64_C(0xF6EF5F915D52BF18),
UINT64_C(0xC210B83AB428155F), UINT64_C(0xAD4F1F390A643BEC),
UINT64_C(0xCC5269926E5AE027), UINT64_C(0xA30DCE91D016CE94),
UINT64_C(0x97F2293A396C64D3), UINT64_C(0xF8AD8E3987204A60),
UINT64_C(0xFF0CE66DEA99305C), UINT64_C(0x9053416E54D51EEF),
UINT64_C(0xA4ACA6C5BDAFB4A8), UINT64_C(0xCBF301C603E39A1B),
UINT64_C(0x7144229E2F3EED56), UINT64_C(0x1E1B859D9172C3E5),
UINT64_C(0x2AE46236780869A2), UINT64_C(0x45BBC535C6444711),
UINT64_C(0x421AAD61ABFD3D2D), UINT64_C(0x2D450A6215B1139E),
UINT64_C(0x19BAEDC9FCCBB9D9), UINT64_C(0x76E54ACA4287976A),
UINT64_C(0x17F83C6126B94CA1), UINT64_C(0x78A79B6298F56212),
UINT64_C(0x4C587CC9718FC855), UINT64_C(0x2307DBCACFC3E6E6),
UINT64_C(0x24A6B39EA27A9CDA), UINT64_C(0x4BF9149D1C36B269),
UINT64_C(0x7F06F336F54C182E), UINT64_C(0x105954354B00369D),
UINT64_C(0x382211CF179F762B), UINT64_C(0x577DB6CCA9D35898),
UINT64_C(0x6382516740A9F2DF), UINT64_C(0x0CDDF664FEE5DC6C),
UINT64_C(0x0B7C9E30935CA650), UINT64_C(0x642339332D1088E3),
UINT64_C(0x50DCDE98C46A22A4), UINT64_C(0x3F83799B7A260C17),
UINT64_C(0x5E9E0F301E18D7DC), UINT64_C(0x31C1A833A054F96F),
UINT64_C(0x053E4F98492E5328), UINT64_C(0x6A61E89BF7627D9B),
UINT64_C(0x6DC080CF9ADB07A7), UINT64_C(0x029F27CC24972914),
UINT64_C(0x3660C067CDED8353), UINT64_C(0x593F676473A1ADE0)
}, {
UINT64_C(0x0000000000000000), UINT64_C(0x0DF1D05C9279E954),
UINT64_C(0x1AE2A1B924F3D2A9), UINT64_C(0x171371E5B68A3BFD),
UINT64_C(0xB1DA4DDC62497DC1), UINT64_C(0xBC2B9D80F0309495),
UINT64_C(0xAB38EC6546BAAF68), UINT64_C(0xA6C93C39D4C3463C),
UINT64_C(0xE7AB9517EE3D2210), UINT64_C(0xEA5A454B7C44CB44),
UINT64_C(0xFD4934AECACEF0B9), UINT64_C(0xF0B8E4F258B719ED),
UINT64_C(0x5671D8CB8C745FD1), UINT64_C(0x5B8008971E0DB685),
UINT64_C(0x4C937972A8878D78), UINT64_C(0x4162A92E3AFE642C),
UINT64_C(0xCE572B2FDC7B4420), UINT64_C(0xC3A6FB734E02AD74),
UINT64_C(0xD4B58A96F8889689), UINT64_C(0xD9445ACA6AF17FDD),
UINT64_C(0x7F8D66F3BE3239E1), UINT64_C(0x727CB6AF2C4BD0B5),
UINT64_C(0x656FC74A9AC1EB48), UINT64_C(0x689E171608B8021C),
UINT64_C(0x29FCBE3832466630), UINT64_C(0x240D6E64A03F8F64),
UINT64_C(0x331E1F8116B5B499), UINT64_C(0x3EEFCFDD84CC5DCD),
UINT64_C(0x9826F3E4500F1BF1), UINT64_C(0x95D723B8C276F2A5),
UINT64_C(0x82C4525D74FCC958), UINT64_C(0x8F358201E685200C),
UINT64_C(0x9CAF565EB8F78840), UINT64_C(0x915E86022A8E6114),
UINT64_C(0x864DF7E79C045AE9), UINT64_C(0x8BBC27BB0E7DB3BD),
UINT64_C(0x2D751B82DABEF581), UINT64_C(0x2084CBDE48C71CD5),
UINT64_C(0x3797BA3BFE4D2728), UINT64_C(0x3A666A676C34CE7C),
UINT64_C(0x7B04C34956CAAA50), UINT64_C(0x76F51315C4B34304),
UINT64_C(0x61E662F0723978F9), UINT64_C(0x6C17B2ACE04091AD),
UINT64_C(0xCADE8E953483D791), UINT64_C(0xC72F5EC9A6FA3EC5),
UINT64_C(0xD03C2F2C10700538), UINT64_C(0xDDCDFF708209EC6C),
UINT64_C(0x52F87D71648CCC60), UINT64_C(0x5F09AD2DF6F52534),
UINT64_C(0x481ADCC8407F1EC9), UINT64_C(0x45EB0C94D206F79D),
UINT64_C(0xE32230AD06C5B1A1), UINT64_C(0xEED3E0F194BC58F5),
UINT64_C(0xF9C0911422366308), UINT64_C(0xF4314148B04F8A5C),
UINT64_C(0xB553E8668AB1EE70), UINT64_C(0xB8A2383A18C80724),
UINT64_C(0xAFB149DFAE423CD9), UINT64_C(0xA24099833C3BD58D),
UINT64_C(0x0489A5BAE8F893B1), UINT64_C(0x097875E67A817AE5),
UINT64_C(0x1E6B0403CC0B4118), UINT64_C(0x139AD45F5E72A84C),
UINT64_C(0x385FADBC70EF1181), UINT64_C(0x35AE7DE0E296F8D5),
UINT64_C(0x22BD0C05541CC328), UINT64_C(0x2F4CDC59C6652A7C),
UINT64_C(0x8985E06012A66C40), UINT64_C(0x8474303C80DF8514),
UINT64_C(0x936741D93655BEE9), UINT64_C(0x9E969185A42C57BD),
UINT64_C(0xDFF438AB9ED23391), UINT64_C(0xD205E8F70CABDAC5),
UINT64_C(0xC5169912BA21E138), UINT64_C(0xC8E7494E2858086C),
UINT64_C(0x6E2E7577FC9B4E50), UINT64_C(0x63DFA52B6EE2A704),
UINT64_C(0x74CCD4CED8689CF9), UINT64_C(0x793D04924A1175AD),
UINT64_C(0xF6088693AC9455A1), UINT64_C(0xFBF956CF3EEDBCF5),
UINT64_C(0xECEA272A88678708), UINT64_C(0xE11BF7761A1E6E5C),
UINT64_C(0x47D2CB4FCEDD2860), UINT64_C(0x4A231B135CA4C134),
UINT64_C(0x5D306AF6EA2EFAC9), UINT64_C(0x50C1BAAA7857139D),
UINT64_C(0x11A3138442A977B1), UINT64_C(0x1C52C3D8D0D09EE5),
UINT64_C(0x0B41B23D665AA518), UINT64_C(0x06B06261F4234C4C),
UINT64_C(0xA0795E5820E00A70), UINT64_C(0xAD888E04B299E324),
UINT64_C(0xBA9BFFE10413D8D9), UINT64_C(0xB76A2FBD966A318D),
UINT64_C(0xA4F0FBE2C81899C1), UINT64_C(0xA9012BBE5A617095),
UINT64_C(0xBE125A5BECEB4B68), UINT64_C(0xB3E38A077E92A23C),
UINT64_C(0x152AB63EAA51E400), UINT64_C(0x18DB666238280D54),
UINT64_C(0x0FC817878EA236A9), UINT64_C(0x0239C7DB1CDBDFFD),
UINT64_C(0x435B6EF52625BBD1), UINT64_C(0x4EAABEA9B45C5285),
UINT64_C(0x59B9CF4C02D66978), UINT64_C(0x54481F1090AF802C),
UINT64_C(0xF2812329446CC610), UINT64_C(0xFF70F375D6152F44),
UINT64_C(0xE8638290609F14B9), UINT64_C(0xE59252CCF2E6FDED),
UINT64_C(0x6AA7D0CD1463DDE1), UINT64_C(0x67560091861A34B5),
UINT64_C(0x7045717430900F48), UINT64_C(0x7DB4A128A2E9E61C),
UINT64_C(0xDB7D9D11762AA020), UINT64_C(0xD68C4D4DE4534974),
UINT64_C(0xC19F3CA852D97289), UINT64_C(0xCC6EECF4C0A09BDD),
UINT64_C(0x8D0C45DAFA5EFFF1), UINT64_C(0x80FD9586682716A5),
UINT64_C(0x97EEE463DEAD2D58), UINT64_C(0x9A1F343F4CD4C40C),
UINT64_C(0x3CD6080698178230), UINT64_C(0x3127D85A0A6E6B64),
UINT64_C(0x2634A9BFBCE45099), UINT64_C(0x2BC579E32E9DB9CD),
UINT64_C(0xF5A054D6CA71FB90), UINT64_C(0xF851848A580812C4),
UINT64_C(0xEF42F56FEE822939), UINT64_C(0xE2B325337CFBC06D),
UINT64_C(0x447A190AA8388651), UINT64_C(0x498BC9563A416F05),
UINT64_C(0x5E98B8B38CCB54F8), UINT64_C(0x536968EF1EB2BDAC),
UINT64_C(0x120BC1C1244CD980), UINT64_C(0x1FFA119DB63530D4),
UINT64_C(0x08E9607800BF0B29), UINT64_C(0x0518B02492C6E27D),
UINT64_C(0xA3D18C1D4605A441), UINT64_C(0xAE205C41D47C4D15),
UINT64_C(0xB9332DA462F676E8), UINT64_C(0xB4C2FDF8F08F9FBC),
UINT64_C(0x3BF77FF9160ABFB0), UINT64_C(0x3606AFA5847356E4),
UINT64_C(0x2115DE4032F96D19), UINT64_C(0x2CE40E1CA080844D),
UINT64_C(0x8A2D32257443C271), UINT64_C(0x87DCE279E63A2B25),
UINT64_C(0x90CF939C50B010D8), UINT64_C(0x9D3E43C0C2C9F98C),
UINT64_C(0xDC5CEAEEF8379DA0), UINT64_C(0xD1AD3AB26A4E74F4),
UINT64_C(0xC6BE4B57DCC44F09), UINT64_C(0xCB4F9B0B4EBDA65D),
UINT64_C(0x6D86A7329A7EE061), UINT64_C(0x6077776E08070935),
UINT64_C(0x7764068BBE8D32C8), UINT64_C(0x7A95D6D72CF4DB9C),
UINT64_C(0x690F0288728673D0), UINT64_C(0x64FED2D4E0FF9A84),
UINT64_C(0x73EDA3315675A179), UINT64_C(0x7E1C736DC40C482D),
UINT64_C(0xD8D54F5410CF0E11), UINT64_C(0xD5249F0882B6E745),
UINT64_C(0xC237EEED343CDCB8), UINT64_C(0xCFC63EB1A64535EC),
UINT64_C(0x8EA4979F9CBB51C0), UINT64_C(0x835547C30EC2B894),
UINT64_C(0x94463626B8488369), UINT64_C(0x99B7E67A2A316A3D),
UINT64_C(0x3F7EDA43FEF22C01), UINT64_C(0x328F0A1F6C8BC555),
UINT64_C(0x259C7BFADA01FEA8), UINT64_C(0x286DABA6487817FC),
UINT64_C(0xA75829A7AEFD37F0), UINT64_C(0xAAA9F9FB3C84DEA4),
UINT64_C(0xBDBA881E8A0EE559), UINT64_C(0xB04B584218770C0D),
UINT64_C(0x1682647BCCB44A31), UINT64_C(0x1B73B4275ECDA365),
UINT64_C(0x0C60C5C2E8479898), UINT64_C(0x0191159E7A3E71CC),
UINT64_C(0x40F3BCB040C015E0), UINT64_C(0x4D026CECD2B9FCB4),
UINT64_C(0x5A111D096433C749), UINT64_C(0x57E0CD55F64A2E1D),
UINT64_C(0xF129F16C22896821), UINT64_C(0xFCD82130B0F08175),
UINT64_C(0xEBCB50D5067ABA88), UINT64_C(0xE63A8089940353DC),
UINT64_C(0xCDFFF96ABA9EEA11), UINT64_C(0xC00E293628E70345),
UINT64_C(0xD71D58D39E6D38B8), UINT64_C(0xDAEC888F0C14D1EC),
UINT64_C(0x7C25B4B6D8D797D0), UINT64_C(0x71D464EA4AAE7E84),
UINT64_C(0x66C7150FFC244579), UINT64_C(0x6B36C5536E5DAC2D),
UINT64_C(0x2A546C7D54A3C801), UINT64_C(0x27A5BC21C6DA2155),
UINT64_C(0x30B6CDC470501AA8), UINT64_C(0x3D471D98E229F3FC),
UINT64_C(0x9B8E21A136EAB5C0), UINT64_C(0x967FF1FDA4935C94),
UINT64_C(0x816C801812196769), UINT64_C(0x8C9D504480608E3D),
UINT64_C(0x03A8D24566E5AE31), UINT64_C(0x0E590219F49C4765),
UINT64_C(0x194A73FC42167C98), UINT64_C(0x14BBA3A0D06F95CC),
UINT64_C(0xB2729F9904ACD3F0), UINT64_C(0xBF834FC596D53AA4),
UINT64_C(0xA8903E20205F0159), UINT64_C(0xA561EE7CB226E80D),
UINT64_C(0xE403475288D88C21), UINT64_C(0xE9F2970E1AA16575),
UINT64_C(0xFEE1E6EBAC2B5E88), UINT64_C(0xF31036B73E52B7DC),
UINT64_C(0x55D90A8EEA91F1E0), UINT64_C(0x5828DAD278E818B4),
UINT64_C(0x4F3BAB37CE622349), UINT64_C(0x42CA7B6B5C1BCA1D),
UINT64_C(0x5150AF3402696251), UINT64_C(0x5CA17F6890108B05),
UINT64_C(0x4BB20E8D269AB0F8), UINT64_C(0x4643DED1B4E359AC),
UINT64_C(0xE08AE2E860201F90), UINT64_C(0xED7B32B4F259F6C4),
UINT64_C(0xFA68435144D3CD39), UINT64_C(0xF799930DD6AA246D),
UINT64_C(0xB6FB3A23EC544041), UINT64_C(0xBB0AEA7F7E2DA915),
UINT64_C(0xAC199B9AC8A792E8), UINT64_C(0xA1E84BC65ADE7BBC),
UINT64_C(0x072177FF8E1D3D80), UINT64_C(0x0AD0A7A31C64D4D4),
UINT64_C(0x1DC3D646AAEEEF29), UINT64_C(0x1032061A3897067D),
UINT64_C(0x9F07841BDE122671), UINT64_C(0x92F654474C6BCF25),
UINT64_C(0x85E525A2FAE1F4D8), UINT64_C(0x8814F5FE68981D8C),
UINT64_C(0x2EDDC9C7BC5B5BB0), UINT64_C(0x232C199B2E22B2E4),
UINT64_C(0x343F687E98A88919), UINT64_C(0x39CEB8220AD1604D),
UINT64_C(0x78AC110C302F0461), UINT64_C(0x755DC150A256ED35),
UINT64_C(0x624EB0B514DCD6C8), UINT64_C(0x6FBF60E986A53F9C),
UINT64_C(0xC9765CD0526679A0), UINT64_C(0xC4878C8CC01F90F4),
UINT64_C(0xD394FD697695AB09), UINT64_C(0xDE652D35E4EC425D)
}, {
UINT64_C(0x0000000000000000), UINT64_C(0xCB6D6A914AE10B3F),
UINT64_C(0x96DBD42295C2177E), UINT64_C(0x5DB6BEB3DF231C41),
UINT64_C(0x2CB7A9452A852FFC), UINT64_C(0xE7DAC3D4606424C3),
UINT64_C(0xBA6C7D67BF473882), UINT64_C(0x710117F6F5A633BD),
UINT64_C(0xDD705D247FA5876A), UINT64_C(0x161D37B535448C55),
UINT64_C(0x4BAB8906EA679014), UINT64_C(0x80C6E397A0869B2B),
UINT64_C(0xF1C7F4615520A896), UINT64_C(0x3AAA9EF01FC1A3A9),
UINT64_C(0x671C2043C0E2BFE8), UINT64_C(0xAC714AD28A03B4D7),
UINT64_C(0xBAE1BA48FE4A0FD5), UINT64_C(0x718CD0D9B4AB04EA),
UINT64_C(0x2C3A6E6A6B8818AB), UINT64_C(0xE75704FB21691394),
UINT64_C(0x9656130DD4CF2029), UINT64_C(0x5D3B799C9E2E2B16),
UINT64_C(0x008DC72F410D3757), UINT64_C(0xCBE0ADBE0BEC3C68),
UINT64_C(0x6791E76C81EF88BF), UINT64_C(0xACFC8DFDCB0E8380),
UINT64_C(0xF14A334E142D9FC1), UINT64_C(0x3A2759DF5ECC94FE),
UINT64_C(0x4B264E29AB6AA743), UINT64_C(0x804B24B8E18BAC7C),
UINT64_C(0xDDFD9A0B3EA8B03D), UINT64_C(0x1690F09A7449BB02),
UINT64_C(0xF1DD7B3ED73AC638), UINT64_C(0x3AB011AF9DDBCD07),
UINT64_C(0x6706AF1C42F8D146), UINT64_C(0xAC6BC58D0819DA79),
UINT64_C(0xDD6AD27BFDBFE9C4), UINT64_C(0x1607B8EAB75EE2FB),
UINT64_C(0x4BB10659687DFEBA), UINT64_C(0x80DC6CC8229CF585),
UINT64_C(0x2CAD261AA89F4152), UINT64_C(0xE7C04C8BE27E4A6D),
UINT64_C(0xBA76F2383D5D562C), UINT64_C(0x711B98A977BC5D13),
UINT64_C(0x001A8F5F821A6EAE), UINT64_C(0xCB77E5CEC8FB6591),
UINT64_C(0x96C15B7D17D879D0), UINT64_C(0x5DAC31EC5D3972EF),
UINT64_C(0x4B3CC1762970C9ED), UINT64_C(0x8051ABE76391C2D2),
UINT64_C(0xDDE71554BCB2DE93), UINT64_C(0x168A7FC5F653D5AC),
UINT64_C(0x678B683303F5E611), UINT64_C(0xACE602A24914ED2E),
UINT64_C(0xF150BC119637F16F), UINT64_C(0x3A3DD680DCD6FA50),
UINT64_C(0x964C9C5256D54E87), UINT64_C(0x5D21F6C31C3445B8),
UINT64_C(0x00974870C31759F9), UINT64_C(0xCBFA22E189F652C6),
UINT64_C(0xBAFB35177C50617B), UINT64_C(0x71965F8636B16A44),
UINT64_C(0x2C20E135E9927605), UINT64_C(0xE74D8BA4A3737D3A),
UINT64_C(0xE2BBF77CAE758C71), UINT64_C(0x29D69DEDE494874E),
UINT64_C(0x7460235E3BB79B0F), UINT64_C(0xBF0D49CF71569030),
UINT64_C(0xCE0C5E3984F0A38D), UINT64_C(0x056134A8CE11A8B2),
UINT64_C(0x58D78A1B1132B4F3), UINT64_C(0x93BAE08A5BD3BFCC),
UINT64_C(0x3FCBAA58D1D00B1B), UINT64_C(0xF4A6C0C99B310024),
UINT64_C(0xA9107E7A44121C65), UINT64_C(0x627D14EB0EF3175A),
UINT64_C(0x137C031DFB5524E7), UINT64_C(0xD811698CB1B42FD8),
UINT64_C(0x85A7D73F6E973399), UINT64_C(0x4ECABDAE247638A6),
UINT64_C(0x585A4D34503F83A4), UINT64_C(0x933727A51ADE889B),
UINT64_C(0xCE819916C5FD94DA), UINT64_C(0x05ECF3878F1C9FE5),
UINT64_C(0x74EDE4717ABAAC58), UINT64_C(0xBF808EE0305BA767),
UINT64_C(0xE2363053EF78BB26), UINT64_C(0x295B5AC2A599B019),
UINT64_C(0x852A10102F9A04CE), UINT64_C(0x4E477A81657B0FF1),
UINT64_C(0x13F1C432BA5813B0), UINT64_C(0xD89CAEA3F0B9188F),
UINT64_C(0xA99DB955051F2B32), UINT64_C(0x62F0D3C44FFE200D),
UINT64_C(0x3F466D7790DD3C4C), UINT64_C(0xF42B07E6DA3C3773),
UINT64_C(0x13668C42794F4A49), UINT64_C(0xD80BE6D333AE4176),
UINT64_C(0x85BD5860EC8D5D37), UINT64_C(0x4ED032F1A66C5608),
UINT64_C(0x3FD1250753CA65B5), UINT64_C(0xF4BC4F96192B6E8A),
UINT64_C(0xA90AF125C60872CB), UINT64_C(0x62679BB48CE979F4),
UINT64_C(0xCE16D16606EACD23), UINT64_C(0x057BBBF74C0BC61C),
UINT64_C(0x58CD05449328DA5D), UINT64_C(0x93A06FD5D9C9D162),
UINT64_C(0xE2A178232C6FE2DF), UINT64_C(0x29CC12B2668EE9E0),
UINT64_C(0x747AAC01B9ADF5A1), UINT64_C(0xBF17C690F34CFE9E),
UINT64_C(0xA987360A8705459C), UINT64_C(0x62EA5C9BCDE44EA3),
UINT64_C(0x3F5CE22812C752E2), UINT64_C(0xF43188B9582659DD),
UINT64_C(0x85309F4FAD806A60), UINT64_C(0x4E5DF5DEE761615F),
UINT64_C(0x13EB4B6D38427D1E), UINT64_C(0xD88621FC72A37621),
UINT64_C(0x74F76B2EF8A0C2F6), UINT64_C(0xBF9A01BFB241C9C9),
UINT64_C(0xE22CBF0C6D62D588), UINT64_C(0x2941D59D2783DEB7),
UINT64_C(0x5840C26BD225ED0A), UINT64_C(0x932DA8FA98C4E635),
UINT64_C(0xCE9B164947E7FA74), UINT64_C(0x05F67CD80D06F14B),
UINT64_C(0xC477EFF95CEB18E3), UINT64_C(0x0F1A8568160A13DC),
UINT64_C(0x52AC3BDBC9290F9D), UINT64_C(0x99C1514A83C804A2),
UINT64_C(0xE8C046BC766E371F), UINT64_C(0x23AD2C2D3C8F3C20),
UINT64_C(0x7E1B929EE3AC2061), UINT64_C(0xB576F80FA94D2B5E),
UINT64_C(0x1907B2DD234E9F89), UINT64_C(0xD26AD84C69AF94B6),
UINT64_C(0x8FDC66FFB68C88F7), UINT64_C(0x44B10C6EFC6D83C8),
UINT64_C(0x35B01B9809CBB075), UINT64_C(0xFEDD7109432ABB4A),
UINT64_C(0xA36BCFBA9C09A70B), UINT64_C(0x6806A52BD6E8AC34),
UINT64_C(0x7E9655B1A2A11736), UINT64_C(0xB5FB3F20E8401C09),
UINT64_C(0xE84D819337630048), UINT64_C(0x2320EB027D820B77),
UINT64_C(0x5221FCF4882438CA), UINT64_C(0x994C9665C2C533F5),
UINT64_C(0xC4FA28D61DE62FB4), UINT64_C(0x0F9742475707248B),
UINT64_C(0xA3E60895DD04905C), UINT64_C(0x688B620497E59B63),
UINT64_C(0x353DDCB748C68722), UINT64_C(0xFE50B62602278C1D),
UINT64_C(0x8F51A1D0F781BFA0), UINT64_C(0x443CCB41BD60B49F),
UINT64_C(0x198A75F26243A8DE), UINT64_C(0xD2E71F6328A2A3E1),
UINT64_C(0x35AA94C78BD1DEDB), UINT64_C(0xFEC7FE56C130D5E4),
UINT64_C(0xA37140E51E13C9A5), UINT64_C(0x681C2A7454F2C29A),
UINT64_C(0x191D3D82A154F127), UINT64_C(0xD2705713EBB5FA18),
UINT64_C(0x8FC6E9A03496E659), UINT64_C(0x44AB83317E77ED66),
UINT64_C(0xE8DAC9E3F47459B1), UINT64_C(0x23B7A372BE95528E),
UINT64_C(0x7E011DC161B64ECF), UINT64_C(0xB56C77502B5745F0),
UINT64_C(0xC46D60A6DEF1764D), UINT64_C(0x0F000A3794107D72),
UINT64_C(0x52B6B4844B336133), UINT64_C(0x99DBDE1501D26A0C),
UINT64_C(0x8F4B2E8F759BD10E), UINT64_C(0x4426441E3F7ADA31),
UINT64_C(0x1990FAADE059C670), UINT64_C(0xD2FD903CAAB8CD4F),
UINT64_C(0xA3FC87CA5F1EFEF2), UINT64_C(0x6891ED5B15FFF5CD),
UINT64_C(0x352753E8CADCE98C), UINT64_C(0xFE4A3979803DE2B3),
UINT64_C(0x523B73AB0A3E5664), UINT64_C(0x9956193A40DF5D5B),
UINT64_C(0xC4E0A7899FFC411A), UINT64_C(0x0F8DCD18D51D4A25),
UINT64_C(0x7E8CDAEE20BB7998), UINT64_C(0xB5E1B07F6A5A72A7),
UINT64_C(0xE8570ECCB5796EE6), UINT64_C(0x233A645DFF9865D9),
UINT64_C(0x26CC1885F29E9492), UINT64_C(0xEDA17214B87F9FAD),
UINT64_C(0xB017CCA7675C83EC), UINT64_C(0x7B7AA6362DBD88D3),
UINT64_C(0x0A7BB1C0D81BBB6E), UINT64_C(0xC116DB5192FAB051),
UINT64_C(0x9CA065E24DD9AC10), UINT64_C(0x57CD0F730738A72F),
UINT64_C(0xFBBC45A18D3B13F8), UINT64_C(0x30D12F30C7DA18C7),
UINT64_C(0x6D67918318F90486), UINT64_C(0xA60AFB1252180FB9),
UINT64_C(0xD70BECE4A7BE3C04), UINT64_C(0x1C668675ED5F373B),
UINT64_C(0x41D038C6327C2B7A), UINT64_C(0x8ABD5257789D2045),
UINT64_C(0x9C2DA2CD0CD49B47), UINT64_C(0x5740C85C46359078),
UINT64_C(0x0AF676EF99168C39), UINT64_C(0xC19B1C7ED3F78706),
UINT64_C(0xB09A0B882651B4BB), UINT64_C(0x7BF761196CB0BF84),
UINT64_C(0x2641DFAAB393A3C5), UINT64_C(0xED2CB53BF972A8FA),
UINT64_C(0x415DFFE973711C2D), UINT64_C(0x8A30957839901712),
UINT64_C(0xD7862BCBE6B30B53), UINT64_C(0x1CEB415AAC52006C),
UINT64_C(0x6DEA56AC59F433D1), UINT64_C(0xA6873C3D131538EE),
UINT64_C(0xFB31828ECC3624AF), UINT64_C(0x305CE81F86D72F90),
UINT64_C(0xD71163BB25A452AA), UINT64_C(0x1C7C092A6F455995),
UINT64_C(0x41CAB799B06645D4), UINT64_C(0x8AA7DD08FA874EEB),
UINT64_C(0xFBA6CAFE0F217D56), UINT64_C(0x30CBA06F45C07669),
UINT64_C(0x6D7D1EDC9AE36A28), UINT64_C(0xA610744DD0026117),
UINT64_C(0x0A613E9F5A01D5C0), UINT64_C(0xC10C540E10E0DEFF),
UINT64_C(0x9CBAEABDCFC3C2BE), UINT64_C(0x57D7802C8522C981),
UINT64_C(0x26D697DA7084FA3C), UINT64_C(0xEDBBFD4B3A65F103),
UINT64_C(0xB00D43F8E546ED42), UINT64_C(0x7B602969AFA7E67D),
UINT64_C(0x6DF0D9F3DBEE5D7F), UINT64_C(0xA69DB362910F5640),
UINT64_C(0xFB2B0DD14E2C4A01), UINT64_C(0x3046674004CD413E),
UINT64_C(0x414770B6F16B7283), UINT64_C(0x8A2A1A27BB8A79BC),
UINT64_C(0xD79CA49464A965FD), UINT64_C(0x1CF1CE052E486EC2),
UINT64_C(0xB08084D7A44BDA15), UINT64_C(0x7BEDEE46EEAAD12A),
UINT64_C(0x265B50F53189CD6B), UINT64_C(0xED363A647B68C654),
UINT64_C(0x9C372D928ECEF5E9), UINT64_C(0x575A4703C42FFED6),
UINT64_C(0x0AECF9B01B0CE297), UINT64_C(0xC181932151EDE9A8)
}, {
UINT64_C(0x0000000000000000), UINT64_C(0xDCA12C225E8AEE1D),
UINT64_C(0xB8435944BC14DD3B), UINT64_C(0x64E27566E29E3326),
UINT64_C(0x7087B2887829BA77), UINT64_C(0xAC269EAA26A3546A),
UINT64_C(0xC8C4EBCCC43D674C), UINT64_C(0x1465C7EE9AB78951),
UINT64_C(0xE00E6511F15274EF), UINT64_C(0x3CAF4933AFD89AF2),
UINT64_C(0x584D3C554D46A9D4), UINT64_C(0x84EC107713CC47C9),
UINT64_C(0x9089D799897BCE98), UINT64_C(0x4C28FBBBD7F12085),
UINT64_C(0x28CA8EDD356F13A3), UINT64_C(0xF46BA2FF6BE5FDBE),
UINT64_C(0x4503C48DC90A304C), UINT64_C(0x99A2E8AF9780DE51),
UINT64_C(0xFD409DC9751EED77), UINT64_C(0x21E1B1EB2B94036A),
UINT64_C(0x35847605B1238A3B), UINT64_C(0xE9255A27EFA96426),
UINT64_C(0x8DC72F410D375700), UINT64_C(0x5166036353BDB91D),
UINT64_C(0xA50DA19C385844A3), UINT64_C(0x79AC8DBE66D2AABE),
UINT64_C(0x1D4EF8D8844C9998), UINT64_C(0xC1EFD4FADAC67785),
UINT64_C(0xD58A13144071FED4), UINT64_C(0x092B3F361EFB10C9),
UINT64_C(0x6DC94A50FC6523EF), UINT64_C(0xB1686672A2EFCDF2),
UINT64_C(0x8A06881B93156098), UINT64_C(0x56A7A439CD9F8E85),
UINT64_C(0x3245D15F2F01BDA3), UINT64_C(0xEEE4FD7D718B53BE),
UINT64_C(0xFA813A93EB3CDAEF), UINT64_C(0x262016B1B5B634F2),
UINT64_C(0x42C263D7572807D4), UINT64_C(0x9E634FF509A2E9C9),
UINT64_C(0x6A08ED0A62471477), UINT64_C(0xB6A9C1283CCDFA6A),
UINT64_C(0xD24BB44EDE53C94C), UINT64_C(0x0EEA986C80D92751),
UINT64_C(0x1A8F5F821A6EAE00), UINT64_C(0xC62E73A044E4401D),
UINT64_C(0xA2CC06C6A67A733B), UINT64_C(0x7E6D2AE4F8F09D26),
UINT64_C(0xCF054C965A1F50D4), UINT64_C(0x13A460B40495BEC9),
UINT64_C(0x774615D2E60B8DEF), UINT64_C(0xABE739F0B88163F2),
UINT64_C(0xBF82FE1E2236EAA3), UINT64_C(0x6323D23C7CBC04BE),
UINT64_C(0x07C1A75A9E223798), UINT64_C(0xDB608B78C0A8D985),
UINT64_C(0x2F0B2987AB4D243B), UINT64_C(0xF3AA05A5F5C7CA26),
UINT64_C(0x974870C31759F900), UINT64_C(0x4BE95CE149D3171D),
UINT64_C(0x5F8C9B0FD3649E4C), UINT64_C(0x832DB72D8DEE7051),
UINT64_C(0xE7CFC24B6F704377), UINT64_C(0x3B6EEE6931FAAD6A),
UINT64_C(0x91131E980D8418A2), UINT64_C(0x4DB232BA530EF6BF),
UINT64_C(0x295047DCB190C599), UINT64_C(0xF5F16BFEEF1A2B84),
UINT64_C(0xE194AC1075ADA2D5), UINT64_C(0x3D3580322B274CC8),
UINT64_C(0x59D7F554C9B97FEE), UINT64_C(0x8576D976973391F3),
UINT64_C(0x711D7B89FCD66C4D), UINT64_C(0xADBC57ABA25C8250),
UINT64_C(0xC95E22CD40C2B176), UINT64_C(0x15FF0EEF1E485F6B),
UINT64_C(0x019AC90184FFD63A), UINT64_C(0xDD3BE523DA753827),
UINT64_C(0xB9D9904538EB0B01), UINT64_C(0x6578BC676661E51C),
UINT64_C(0xD410DA15C48E28EE), UINT64_C(0x08B1F6379A04C6F3),
UINT64_C(0x6C538351789AF5D5), UINT64_C(0xB0F2AF7326101BC8),
UINT64_C(0xA497689DBCA79299), UINT64_C(0x783644BFE22D7C84),
UINT64_C(0x1CD431D900B34FA2), UINT64_C(0xC0751DFB5E39A1BF),
UINT64_C(0x341EBF0435DC5C01), UINT64_C(0xE8BF93266B56B21C),
UINT64_C(0x8C5DE64089C8813A), UINT64_C(0x50FCCA62D7426F27),
UINT64_C(0x44990D8C4DF5E676), UINT64_C(0x983821AE137F086B),
UINT64_C(0xFCDA54C8F1E13B4D), UINT64_C(0x207B78EAAF6BD550),
UINT64_C(0x1B1596839E91783A), UINT64_C(0xC7B4BAA1C01B9627),
UINT64_C(0xA356CFC72285A501), UINT64_C(0x7FF7E3E57C0F4B1C),
UINT64_C(0x6B92240BE6B8C24D), UINT64_C(0xB7330829B8322C50),
UINT64_C(0xD3D17D4F5AAC1F76), UINT64_C(0x0F70516D0426F16B),
UINT64_C(0xFB1BF3926FC30CD5), UINT64_C(0x27BADFB03149E2C8),
UINT64_C(0x4358AAD6D3D7D1EE), UINT64_C(0x9FF986F48D5D3FF3),
UINT64_C(0x8B9C411A17EAB6A2), UINT64_C(0x573D6D38496058BF),
UINT64_C(0x33DF185EABFE6B99), UINT64_C(0xEF7E347CF5748584),
UINT64_C(0x5E16520E579B4876), UINT64_C(0x82B77E2C0911A66B),
UINT64_C(0xE6550B4AEB8F954D), UINT64_C(0x3AF42768B5057B50),
UINT64_C(0x2E91E0862FB2F201), UINT64_C(0xF230CCA471381C1C),
UINT64_C(0x96D2B9C293A62F3A), UINT64_C(0x4A7395E0CD2CC127),
UINT64_C(0xBE18371FA6C93C99), UINT64_C(0x62B91B3DF843D284),
UINT64_C(0x065B6E5B1ADDE1A2), UINT64_C(0xDAFA427944570FBF),
UINT64_C(0xCE9F8597DEE086EE), UINT64_C(0x123EA9B5806A68F3),
UINT64_C(0x76DCDCD362F45BD5), UINT64_C(0xAA7DF0F13C7EB5C8),
UINT64_C(0xA739329F30A7E9D6), UINT64_C(0x7B981EBD6E2D07CB),
UINT64_C(0x1F7A6BDB8CB334ED), UINT64_C(0xC3DB47F9D239DAF0),
UINT64_C(0xD7BE8017488E53A1), UINT64_C(0x0B1FAC351604BDBC),
UINT64_C(0x6FFDD953F49A8E9A), UINT64_C(0xB35CF571AA106087),
UINT64_C(0x4737578EC1F59D39), UINT64_C(0x9B967BAC9F7F7324),
UINT64_C(0xFF740ECA7DE14002), UINT64_C(0x23D522E8236BAE1F),
UINT64_C(0x37B0E506B9DC274E), UINT64_C(0xEB11C924E756C953),
UINT64_C(0x8FF3BC4205C8FA75), UINT64_C(0x535290605B421468),
UINT64_C(0xE23AF612F9ADD99A), UINT64_C(0x3E9BDA30A7273787),
UINT64_C(0x5A79AF5645B904A1), UINT64_C(0x86D883741B33EABC),
UINT64_C(0x92BD449A818463ED), UINT64_C(0x4E1C68B8DF0E8DF0),
UINT64_C(0x2AFE1DDE3D90BED6), UINT64_C(0xF65F31FC631A50CB),
UINT64_C(0x0234930308FFAD75), UINT64_C(0xDE95BF2156754368),
UINT64_C(0xBA77CA47B4EB704E), UINT64_C(0x66D6E665EA619E53),
UINT64_C(0x72B3218B70D61702), UINT64_C(0xAE120DA92E5CF91F),
UINT64_C(0xCAF078CFCCC2CA39), UINT64_C(0x165154ED92482424),
UINT64_C(0x2D3FBA84A3B2894E), UINT64_C(0xF19E96A6FD386753),
UINT64_C(0x957CE3C01FA65475), UINT64_C(0x49DDCFE2412CBA68),
UINT64_C(0x5DB8080CDB9B3339), UINT64_C(0x8119242E8511DD24),
UINT64_C(0xE5FB5148678FEE02), UINT64_C(0x395A7D6A3905001F),
UINT64_C(0xCD31DF9552E0FDA1), UINT64_C(0x1190F3B70C6A13BC),
UINT64_C(0x757286D1EEF4209A), UINT64_C(0xA9D3AAF3B07ECE87),
UINT64_C(0xBDB66D1D2AC947D6), UINT64_C(0x6117413F7443A9CB),
UINT64_C(0x05F5345996DD9AED), UINT64_C(0xD954187BC85774F0),
UINT64_C(0x683C7E096AB8B902), UINT64_C(0xB49D522B3432571F),
UINT64_C(0xD07F274DD6AC6439), UINT64_C(0x0CDE0B6F88268A24),
UINT64_C(0x18BBCC8112910375), UINT64_C(0xC41AE0A34C1BED68),
UINT64_C(0xA0F895C5AE85DE4E), UINT64_C(0x7C59B9E7F00F3053),
UINT64_C(0x88321B189BEACDED), UINT64_C(0x5493373AC56023F0),
UINT64_C(0x3071425C27FE10D6), UINT64_C(0xECD06E7E7974FECB),
UINT64_C(0xF8B5A990E3C3779A), UINT64_C(0x241485B2BD499987),
UINT64_C(0x40F6F0D45FD7AAA1), UINT64_C(0x9C57DCF6015D44BC),
UINT64_C(0x362A2C073D23F174), UINT64_C(0xEA8B002563A91F69),
UINT64_C(0x8E69754381372C4F), UINT64_C(0x52C85961DFBDC252),
UINT64_C(0x46AD9E8F450A4B03), UINT64_C(0x9A0CB2AD1B80A51E),
UINT64_C(0xFEEEC7CBF91E9638), UINT64_C(0x224FEBE9A7947825),
UINT64_C(0xD6244916CC71859B), UINT64_C(0x0A85653492FB6B86),
UINT64_C(0x6E671052706558A0), UINT64_C(0xB2C63C702EEFB6BD),
UINT64_C(0xA6A3FB9EB4583FEC), UINT64_C(0x7A02D7BCEAD2D1F1),
UINT64_C(0x1EE0A2DA084CE2D7), UINT64_C(0xC2418EF856C60CCA),
UINT64_C(0x7329E88AF429C138), UINT64_C(0xAF88C4A8AAA32F25),
UINT64_C(0xCB6AB1CE483D1C03), UINT64_C(0x17CB9DEC16B7F21E),
UINT64_C(0x03AE5A028C007B4F), UINT64_C(0xDF0F7620D28A9552),
UINT64_C(0xBBED03463014A674), UINT64_C(0x674C2F646E9E4869),
UINT64_C(0x93278D9B057BB5D7), UINT64_C(0x4F86A1B95BF15BCA),
UINT64_C(0x2B64D4DFB96F68EC), UINT64_C(0xF7C5F8FDE7E586F1),
UINT64_C(0xE3A03F137D520FA0), UINT64_C(0x3F01133123D8E1BD),
UINT64_C(0x5BE36657C146D29B), UINT64_C(0x87424A759FCC3C86),
UINT64_C(0xBC2CA41CAE3691EC), UINT64_C(0x608D883EF0BC7FF1),
UINT64_C(0x046FFD5812224CD7), UINT64_C(0xD8CED17A4CA8A2CA),
UINT64_C(0xCCAB1694D61F2B9B), UINT64_C(0x100A3AB68895C586),
UINT64_C(0x74E84FD06A0BF6A0), UINT64_C(0xA84963F2348118BD),
UINT64_C(0x5C22C10D5F64E503), UINT64_C(0x8083ED2F01EE0B1E),
UINT64_C(0xE4619849E3703838), UINT64_C(0x38C0B46BBDFAD625),
UINT64_C(0x2CA57385274D5F74), UINT64_C(0xF0045FA779C7B169),
UINT64_C(0x94E62AC19B59824F), UINT64_C(0x484706E3C5D36C52),
UINT64_C(0xF92F6091673CA1A0), UINT64_C(0x258E4CB339B64FBD),
UINT64_C(0x416C39D5DB287C9B), UINT64_C(0x9DCD15F785A29286),
UINT64_C(0x89A8D2191F151BD7), UINT64_C(0x5509FE3B419FF5CA),
UINT64_C(0x31EB8B5DA301C6EC), UINT64_C(0xED4AA77FFD8B28F1),
UINT64_C(0x19210580966ED54F), UINT64_C(0xC58029A2C8E43B52),
UINT64_C(0xA1625CC42A7A0874), UINT64_C(0x7DC370E674F0E669),
UINT64_C(0x69A6B708EE476F38), UINT64_C(0xB5079B2AB0CD8125),
UINT64_C(0xD1E5EE4C5253B203), UINT64_C(0x0D44C26E0CD95C1E)
}
};
