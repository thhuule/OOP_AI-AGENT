import path from "node:path";
import { FileBlob, PresentationFile } from "@oai/artifact-tool";

const root = "C:\\Code\\Code\\OOP\\AI-AGENT project";
const tmp = path.join(root, ".codex-ppt-format");
const input = path.join(tmp, "template-starter.pptx");
const output = path.join(root, "AI_Agent_Framework_Presentation_Formatted.pptx");

const presentation = await PresentationFile.importPptx(await FileBlob.load(input));
const snapshot = await presentation.inspect({
  kind: "slide,textbox,shape,notes,layout",
  include: "id,slide,name,text,textPreview,bbox",
  maxChars: 250000,
});
const records = snapshot.ndjson.split(/\r?\n/).filter(Boolean).map(JSON.parse);

for (const record of records) {
  if (record.kind !== "textbox" || !record.bbox) continue;

  if (record.slide >= 2 && record.slide <= 15 && record.bbox[1] < 120) {
    presentation.resolve(record.id).position = {
      left: 103.27,
      top: 68.65,
      width: 1710.15,
      height: 99.48,
    };
  }

  if (record.slide >= 2 && record.slide <= 17 && record.bbox[1] > 950) {
    const shape = presentation.resolve(record.id);
    if (String(record.text || "").startsWith("Role ")) {
      shape.position = { left: 107.6, top: 991.97, width: 470.78, height: 33.63 };
    } else if (String(record.text || "") === String(record.slide)) {
      shape.position = { left: 1726, top: 991.97, width: 80.22, height: 33.63 };
      shape.text = record.slide >= 10 ? `1\u2009${record.slide - 10}` : String(record.slide);
      shape.text.style = {
        fontSize: 22.01,
        typeface: "Aptos",
        color: "#A9B5C7",
        alignment: "right",
        verticalAlignment: "top",
        autoFit: "resizeShapeToFitText",
        insets: { top: 0, right: 0, bottom: 0, left: 0 },
      };
    }
  }
}

const subtitle = records.find(
  (record) => record.kind === "textbox" && record.slide === 2 && String(record.text || "").endsWith(" ###"),
);
if (!subtitle) throw new Error("Roadmap subtitle was not found.");
presentation.resolve(subtitle.id).text.replace(" ###", "");

const pptx = await PresentationFile.exportPptx(presentation);
await pptx.save(output);
console.log(output);
